//===-- LC32ISelDAGToDAG.cpp - Instruction Selector for LC-3.2 ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelDAGToDAG.h"
#include "LC32CLOpts.h"
#include "LC32TargetMachine.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/Support/ErrorHandling.h"
#include <cstdint>
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32ISelDAGToDag"

char LC32DAGToDAGISel::ID;

INITIALIZE_PASS(LC32DAGToDAGISel, DEBUG_TYPE,
                "LC-3.2 DAG->DAG Instruction Selection", false, false)

// Provides: SelectCode
#define GET_DAGISEL_BODY LC32DAGToDAGISel
#include "LC32GenDAGISel.inc"

void LC32DAGToDAGISel::Select(SDNode *N) {
  // Custom pattern matching
  if (this->SelectRepeatedXorShift(N))
    return;
  if (this->SelectRepeatedAdd(N))
    return;
  if (this->SelectRepeatedShift(N))
    return;
  // TableGen
  this->SelectCode(N);
}

bool LC32DAGToDAGISel::SelectRepeatedXorShift(SDNode *N) {
  // Populate variables
  SDLoc dl(N);

  // Input validation
  // Note that type legalization has already happened by this point, so we can
  // assume the input is a legal type.
  if (N->getOpcode() != ISD::Constant)
    return false;
  assert(N->getValueType(0) == MVT::i32 && "Constants should be i32");

  // If we're not allowed to use repeated operations, bail
  if (MaxRepeatedOps == 0)
    return false;

  // Get the value we're trying to put
  // Make sure the value is 32-bit sign exteded
  int64_t imm = SignExtend64<32>(cast<ConstantSDNode>(N)->getSExtValue());

  // Edge case if we're trying to zero out
  if (imm == 0) {
    SDNode *out = this->CurDAG->getMachineNode(LC32::C_LOADZERO, dl, MVT::i32);
    this->ReplaceNode(N, out);
    return true;
  }

  // We'll figure out what instructions we need to execute to materialize the
  // constant into a zeroed register. This way, we can compute how many
  // instructions it'll take before constructing the nodes.

  // The two different instructions ("commands") we can have
  typedef enum command_id_t {
    XOR,
    LSHF,
  } command_id_t;
  // Structure describing commands
  typedef struct command_t {
    command_id_t id;
    int64_t arg;
  } command_t;

  // Compute the commands we'll need to execute
  // List is in REVERSE order
  SmallVector<command_t, 14> commands;
  {
    int64_t togo = imm;
    size_t loopcnt = 0;
    while (togo != 0) {
      // We should never have to loop more than fourteen times to materialize
      // any immediate. If we do, something went wrong.
      assert(loopcnt < 14 && "Looped too many times");
      loopcnt++;

      // Run a greedy algorithm to compute the commands. If we can XOR, do that.
      // Otherwise, shift down.

      // Check if there is some value in the LSB 5 bits
      int64_t lsb5 = SignExtend64<5>(togo & 0x1f);
      if (lsb5 != 0) {
        // XOR with it
        commands.push_back(command_t{XOR, lsb5});
        // Propagate
        togo ^= lsb5;
        // Check we won't go into this branch on the next loop
        assert(SignExtend64<5>(togo & 0x1f) == 0 && "Didn't clear LSB 5 bits");

      } else {

        // The LSB 5 bits are clear, but we can shift by up to 8. Therefore,
        // compute how many of the bottom bits are actually clear. We do this
        // indirectly, by looking for the lowest index with a one.
        size_t min_one_index;
        for (min_one_index = 0; min_one_index < 32; min_one_index++) {
          // If what we're claiming is the minimum index of a one is actually a
          // zero, continue on. The actual index is higher.
          if ((togo & (1 << min_one_index)) == 0)
            continue;
          // Otherwise, we fould the minimum index
          break;
        }
        assert(min_one_index >= 5 && "LSB 5 bits should've been clear");
        assert(min_one_index < 32 && "Should have at least one bit set");

        // Compute the actual shift amount
        int64_t shamt = min_one_index >= 8 ? 8 : min_one_index;
        // Shift out everything that's zero
        commands.push_back(command_t{LSHF, shamt});
        // Propagate
        togo >>= shamt;
      }
    }
  }
  // We should have some commands after this
  assert(commands.size() != 0 && "Input to loop should not have been zero");

  // Check whether we're allowed to do that many shifts and xors. Also remember
  // to count the initial clearing.
  if (commands.size() + 1 > MaxRepeatedOps)
    return false;

  // Execute the commands
  SDNode *out = this->CurDAG->getMachineNode(LC32::C_LOADZERO, dl, MVT::i32);
  while (!commands.empty()) {
    // Pull the command
    command_t cmd = commands.pop_back_val();
    // Execute
    switch (cmd.id) {
    case XOR:
      out = this->CurDAG->getMachineNode(
          LC32::XORi, dl, MVT::i32, SDValue(out, 0),
          this->CurDAG->getTargetConstant(cmd.arg, dl, MVT::i32));
      break;
    case LSHF:
      out = this->CurDAG->getMachineNode(
          LC32::LSHFi, dl, MVT::i32, SDValue(out, 0),
          this->CurDAG->getTargetConstant(cmd.arg, dl, MVT::i32));
      break;
    default:
      llvm_unreachable("Unhandled case");
    }
  }

  // Done
  this->ReplaceNode(N, out);
  return true;
}

bool LC32DAGToDAGISel::SelectRepeatedAdd(SDNode *N) {
  // Populate variables
  SDLoc dl(N);

  // Input validation
  // Note that type legalization has already happened by this point, so we can
  // assume the input is a legal type.
  if (N->getOpcode() != ISD::Constant)
    return false;
  assert(N->getValueType(0) == MVT::i32 && "Constants should be i32");

  // If we're not allowed to use repeated operations, bail
  if (MaxRepeatedOps == 0)
    return false;

  // Check whether we should use repeated ADDs
  // Note the -1 since we need one instruction to zero it out. That's also why
  // we checked MaxRepeatedOps != 0
  int64_t imm = cast<ConstantSDNode>(N)->getSExtValue();
  if (imm < INT64_C(-16) * static_cast<int64_t>(MaxRepeatedOps - 1) ||
      imm > INT64_C(15) * static_cast<int64_t>(MaxRepeatedOps - 1))
    return false;

  // Start with zero
  SDNode *out = this->CurDAG->getMachineNode(LC32::C_LOADZERO, dl, MVT::i32);
  // Do repeated ADDi
  {
    int64_t to_go = imm;
    while (to_go != 0) {
      int64_t to_add = std::max(INT64_C(-16), std::min(INT64_C(15), to_go));
      out = this->CurDAG->getMachineNode(
          LC32::ADDi, dl, MVT::i32, SDValue(out, 0),
          this->CurDAG->getTargetConstant(to_add, dl, MVT::i32));
      to_go -= to_add;
    }
  }
  // Done
  this->ReplaceNode(N, out);
  return true;
}

bool LC32DAGToDAGISel::SelectRepeatedShift(SDNode *N) {
  // Populate variables
  SDLoc dl(N);

  // If the opcode isn't something we can handle, bail
  if (N->getOpcode() != ISD::SHL && N->getOpcode() != ISD::SRL &&
      N->getOpcode() != ISD::SRA)
    return false;
  // If the RHS isn't a constant, bail
  // Assert that we have an RHS
  assert(N->getNumOperands() == 2 && "Bad number of operands for shift");
  if (N->getOperand(1)->getOpcode() != ISD::Constant)
    return false;
  // Assert the output and the constant have legal types
  // Type legalization has already happened, so we can make this assumption
  assert(N->getValueType(0) == MVT::i32 && "Results should be i32");
  assert(N->getOperand(1).getValueType() == MVT::i32 &&
         "Constants should be i32");

  // Get the target node to use given the opcode
  unsigned target_op;
  switch (N->getOpcode()) {
  case ISD::SHL:
    target_op = LC32::LSHFi;
    break;
  case ISD::SRL:
    target_op = LC32::RSHFLi;
    break;
  case ISD::SRA:
    target_op = LC32::RSHFAi;
    break;
  default:
    llvm_unreachable("Unhandled case");
  }

  // Pull out the immediate value
  // Assert that it is in a reasonable range
  uint64_t imm = cast<ConstantSDNode>(N->getOperand(1))->getZExtValue();
  assert(imm > 0 && imm < 32 && "Bad value for shift immediate");
  // Check that we're allowed to do enough repetitions
  if (MaxRepeatedShf < (imm + 7) / 8)
    return false;

  // Loop variables
  // The variable out will be populated on the first iteration
  SDNode *out = nullptr;
  uint64_t to_go = imm;
  while (to_go != 0) {

    // Compute what value to use as the base for this iteration
    // It's either the result of the last iteration or the input to this
    // function
    SDValue base = out == nullptr ? N->getOperand(0) : SDValue(out, 0);

    // Compute how much we have to shift by this iteration
    uint64_t to_shf = std::min(to_go, UINT64_C(8));
    // Update out
    out = this->CurDAG->getMachineNode(
        target_op, dl, MVT::i32, base,
        this->CurDAG->getTargetConstant(to_shf, dl, MVT::i32));
    // Update the amount to go
    to_go -= to_shf;
  }
  assert(out != nullptr);

  // Done
  this->ReplaceNode(N, out);
  return true;
}
