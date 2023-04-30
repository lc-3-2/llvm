//===-- LC32ISelDAGToDAG.cpp - Instruction Selector for LC-3.2 ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass does instruction selection on the DAG. It's pattern matching. Quite
// a lot of this is handled by TableGen.
// See: LC32GenDAGISel.inc
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H
#define LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H

#include "LC32TargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"

namespace llvm {

class LC32DAGToDAGISel : public SelectionDAGISel {
public:
  static char ID;
  LC32DAGToDAGISel() = delete;
  LC32DAGToDAGISel(LC32TargetMachine &TM, CodeGenOpt::Level OptLevel)
      : SelectionDAGISel(this->ID, TM, OptLevel) {}

  StringRef getPassName() const override {
    return "LC-3.2 DAG->DAG Instruction Selection";
  }

private:
#define GET_DAGISEL_DECL
#include "LC32GenDAGISel.inc"

  void Select(SDNode *N) override;

  // See: td/instr/LC32MemInstrInfo.td
  bool SelectFrameIndex(SDValue In, SDValue &Out);
};

// Required for INITIALIZE_PASS
void initializeLC32DAGToDAGISelPass(PassRegistry &);

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H
