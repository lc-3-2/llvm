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

  /**
   * The address of the pointer used for sret
   *
   * This is populated on call to the function, and it is put onto the return
   * address stack in lieu of a return value for functions that are sret.
   */
  Register SRetAddrReg;

  /**
   * The frame index to use when spilling a register for branch relaxation
   *
   * This is populated by `processFunctionBeforeFrameFinalized` if it computes
   * that we may require branch relaxation to make sure all the branches are in
   * range. It is used by `insertIndirectBranch` to do just that. Specifically,
   * it will spill AT into this frame index so it can use it as a scratchpad to
   * put the target address into.
   */
  int BranchRelaxationFI;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32MACHINEFUNCTIONINFO_H
