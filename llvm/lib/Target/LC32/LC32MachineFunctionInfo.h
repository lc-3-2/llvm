//=== LC32MachineFunctionInfo.h - LC-3.2 Machine Function Info ---*- C++ -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module can be derived by targets to hold any information they see fit.
// They would do this if some information needs to be transferred between
// different modules, and if that information is on a per-function basis.
//
// In other words, this is a container for miscellaneous data.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32MACHINEFUNCTIONINFO_H
#define LLVM_LIB_TARGET_LC32_LC32MACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

class LC32MachineFunctionInfo : public MachineFunctionInfo {
public:
  LC32MachineFunctionInfo();
  LC32MachineFunctionInfo(const Function &F, const TargetSubtargetInfo *STI);
  MachineFunctionInfo *
  clone(BumpPtrAllocator &Allocator, MachineFunction &DestMF,
        const DenseMap<MachineBasicBlock *, MachineBasicBlock *> &Src2DstMBB)
      const override;

  /**
   * The frame index corresponding to the first variadic argument
   *
   * This is populated in the call frame setup code, and it is read whenever
   * varargs are used.
   */
  int VarArgsFI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32MACHINEFUNCTIONINFO_H
