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
//
// Also, all of the implementation is in the C++ file. We only expose a way to
// create the pass. This is because the header information has a lot of
// implementation code.
//
// See: LC32GenDAGISel.inc
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H
#define LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H

#include "LC32TargetMachine.h"
#include "llvm/Pass.h"

namespace llvm {

// Interface to create the pass
FunctionPass *createLC32ISelDag(LC32TargetMachine &TM,
                                CodeGenOpt::Level OptLevel);

// Required for INITIALIZE_PASS
void initializeLC32DAGToDAGISelPass(PassRegistry &);

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32ISELDAGTODAG_H
