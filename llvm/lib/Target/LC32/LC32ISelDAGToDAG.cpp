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
// Requires: SelectFrameIndex
// Requires: SelectGlobalAddress
// Requires: SelectExternalSymbol
// Requires: SelectConstantPool
// Requires: SelectJumpTable
// Requires: SelectBlockAddress
#define GET_DAGISEL_BODY LC32DAGToDAGISel
#include "LC32GenDAGISel.inc"

void LC32DAGToDAGISel::Select(SDNode *N) {
  // Custom pattern matching
  if (this->SelectRepeatedAdd(N))
    return;
  // TableGen
  this->SelectCode(N);
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

  // Check whether we should use repeated ADDs
  // Match zeros too, even though we have a pattern for that
  int64_t imm = cast<ConstantSDNode>(N)->getSExtValue();
  if (imm < -16l * static_cast<int64_t>(MaxRepeatedAdd.getValue()) ||
      imm > 15l * static_cast<int64_t>(MaxRepeatedAdd.getValue()))
    return false;

  // Start with zero
  SDNode *out = this->CurDAG->getMachineNode(LC32::C_MOVE_ZERO, dl, MVT::i32);
  // Do repeated ADDi
  {
    int64_t to_go = imm;
    while (to_go != 0) {
      int64_t to_add = std::max(-16l, std::min(15l, to_go));
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

bool LC32DAGToDAGISel::SelectFrameIndex(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::FrameIndex)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetFrameIndex(
      cast<FrameIndexSDNode>(In)->getIndex(), MVT::i32);
  return true;
}

bool LC32DAGToDAGISel::SelectGlobalAddress(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::GlobalAddress)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetGlobalAddress(
      cast<GlobalAddressSDNode>(In)->getGlobal(), SDLoc(In), MVT::i32,
      cast<GlobalAddressSDNode>(In)->getOffset());
  return true;
}

bool LC32DAGToDAGISel::SelectExternalSymbol(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::ExternalSymbol)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetExternalSymbol(
      cast<ExternalSymbolSDNode>(In)->getSymbol(), MVT::i32);
  return true;
}

bool LC32DAGToDAGISel::SelectConstantPool(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::ConstantPool)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetConstantPool(
      cast<ConstantPoolSDNode>(In)->getConstVal(), MVT::i32,
      cast<ConstantPoolSDNode>(In)->getAlign(),
      cast<ConstantPoolSDNode>(In)->getOffset());
  return true;
}

bool LC32DAGToDAGISel::SelectJumpTable(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::JumpTable)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetJumpTable(cast<JumpTableSDNode>(In)->getIndex(),
                                         MVT::i32);
  return true;
}

bool LC32DAGToDAGISel::SelectBlockAddress(SDValue In, SDValue &Out) {
  if (In.getOpcode() != ISD::BlockAddress)
    return false;
  assert(In.getValueType() == MVT::i32 && "Addresses should be i32");
  Out = this->CurDAG->getTargetBlockAddress(
      cast<BlockAddressSDNode>(In)->getBlockAddress(), MVT::i32,
      cast<BlockAddressSDNode>(In)->getOffset());
  return true;
}
