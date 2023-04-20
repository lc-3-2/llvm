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
#define PASS_NAME "LC-3.2 DAG->DAG Instruction Selection"

namespace {

class LC32DAGToDAGISel : public SelectionDAGISel {
public:
  static char ID;
  LC32DAGToDAGISel() = delete;
  LC32DAGToDAGISel(LC32TargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(this->ID, TM, OptLevel) {}

private:
#include "LC32GenDAGISel.inc"

  void Select(SDNode *N) override;
};

} // namespace

char LC32DAGToDAGISel::ID;

INITIALIZE_PASS(LC32DAGToDAGISel, DEBUG_TYPE, PASS_NAME, false, false)

FunctionPass *llvm::createLC32ISelDag(LC32TargetMachine &TM,
                                      CodeGenOpt::Level OptLevel) {
  return new LC32DAGToDAGISel(TM, OptLevel);
}

void LC32DAGToDAGISel::Select(SDNode *N) { llvm_unreachable("UNIMPLEMENTED"); }
