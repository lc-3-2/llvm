//=== LC32MachineFunctionInfo.cpp - LC-3.2 Machine Function Info *- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32MachineFunctionInfo.h"
using namespace llvm;
#define DEBUG_TYPE "LC32MachineFunctionInfo"

LC32MachineFunctionInfo::LC32MachineFunctionInfo() = default;

LC32MachineFunctionInfo::LC32MachineFunctionInfo(const Function &F,
                                                 const TargetSubtargetInfo *STI)
    : VarArgsFI(0), SRetAddrReg(0), BranchRelaxationFI(-1) {}

MachineFunctionInfo *LC32MachineFunctionInfo::clone(
    BumpPtrAllocator &Allocator, MachineFunction &DestMF,
    const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
    const {
  return DestMF.cloneInfo<LC32MachineFunctionInfo>(*this);
}
