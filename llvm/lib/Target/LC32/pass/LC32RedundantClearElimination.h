//==- LC32RedundantClearElimination.h - Remove Register Clears --*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This pass is based off `RISCVRedundantCopyElimination`, and most of the logic
// was taken from there.
//
// The idea is to remove unnecessary register clears after branches. For
// example, we might have
// ```
// bb0:
//   C_BR_CMP_ZERO 2, $r0, %bb1
// bb1:
//   $r0 = C_MOVE_ZERO
// ```
// In this case, the `C_MOVE_ZERO` instruction is redundant since we know the
// register is already zero as a result of the branch. LLVM can't handle this
// since it does optimization on a per-basic-block level. Thus, we handle it.
//
// The AArch64 version of this pass also handles immediates. We don't because of
// the difficulty in doing that for this target, especially since we don't pack
// immediates with our branches.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_PASS_LC32REDUNDANTCLEARELIMNINATION_H
#define LLVM_LIB_TARGET_LC32_PASS_LC32REDUNDANTCLEARELIMNINATION_H

#include "llvm/CodeGen/MachineFunctionPass.h"

namespace llvm {

// Required for INITIALIZE_PASS
void initializeLC32RedundantClearEliminationPass(PassRegistry &);

class LC32RedundantClearElimination : public MachineFunctionPass {
public:
  static char ID;
  LC32RedundantClearElimination() : MachineFunctionPass(this->ID) {
    initializeLC32RedundantClearEliminationPass(
        *PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override {
    return "LC-3.2 Redundant Register Clear Elimination";
  }
  MachineFunctionProperties getRequiredProperties() const override {
    return MachineFunctionProperties().set(
        MachineFunctionProperties::Property::NoVRegs);
  }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_PASS_LC32REDUNDANTCLEARELIMNINATION_H
