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
// Requires: SelectIncrementedImm5
// Requires: SelectIncrementedImm16
// Requires: SelectIncrementedImm32
// Requires: SelectInvertedImm5
// Requires: SelectInvertedImm16
// Requires: SelectInvertedImm32
#define GET_DAGISEL_BODY LC32DAGToDAGISel
#include "LC32GenDAGISel.inc"

void LC32DAGToDAGISel::Select(SDNode *N) { this->SelectCode(N); }

bool LC32DAGToDAGISel::SelectFrameIndex(SDValue In, SDValue &Out) {
  // Check we actually got a frameindex
  if (In.getOpcode() != ISD::FrameIndex)
    return false;
  // Convert to a TargetFrameIndex
  Out = this->CurDAG->getTargetFrameIndex(
      cast<FrameIndexSDNode>(In)->getIndex(), MVT::i32);
  return true;
}
