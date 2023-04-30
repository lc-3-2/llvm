//===-- LC32ISelDAGToDAG.cpp - Instruction Selector for LC-3.2 ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelDAGToDAG.h"
#include "LC32TargetMachine.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
using namespace llvm;
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

void LC32DAGToDAGISel::Select(SDNode *N) { this->SelectCode(N); }

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
