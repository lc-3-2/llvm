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

  // Compute what values we will need to XOR with
  // In REVERSE order
  SmallVector<int64_t> xor_values;
  {
    int64_t togo = imm;
    size_t loopcnt = 0;
    while (togo != 0) {
      // We should never have to loop more than seven times
      assert(loopcnt < 7 && "Looped too many times");
      loopcnt++;

      // Extract the least significant five bits
      int64_t lsb5 = SignExtend64<5>(togo & 0x1f);
      togo >>= 5;
      xor_values.push_back(lsb5);

      // If we are going to XOR with a negative value, account for sign
      // extension
      if (lsb5 < 0)
        togo = ~togo;
    }
  }
  // We shouldn't have gotten zero
  assert(xor_values.size() != 0 && "Input to loop should not have been zero");

  // Check whether we're allowed to do that many shifts and xors
  // Usually, each would take two instructions. If we're XORing with zero, it
  // only counts one for the shift
  {
    size_t instcnt = 0;
    for (int64_t &v : xor_values)
      instcnt += v == 0 ? 1 : 2;
    if (instcnt > MaxRepeatedOps)
      return false;
  }

  // Do the XORs
  SDNode *out = nullptr;
  while (!xor_values.empty()) {
    int64_t toxor = xor_values.pop_back_val();
    // Compute what value to use as the base for this iteration
    // It's either shifting the last iteration, or zeroing out if this is the
    // first iteration
    SDNode *base =
        out == nullptr
            ? this->CurDAG->getMachineNode(LC32::C_LOADZERO, dl, MVT::i32)
            : this->CurDAG->getMachineNode(
                  LC32::LSHFi, dl, MVT::i32, SDValue(out, 0),
                  this->CurDAG->getTargetConstant(5, dl, MVT::i32));
    // XOR
    // Skip this if toxor == 0
    if (toxor != 0)
      out = this->CurDAG->getMachineNode(
          LC32::XORi, dl, MVT::i32, SDValue(base, 0),
          this->CurDAG->getTargetConstant(toxor, dl, MVT::i32));
    else
      out = base;
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
