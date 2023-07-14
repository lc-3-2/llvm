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

private:
  /**
   * Run the analysis on a per-basic-block basis
   *
   * @param[in] MBB The block to run on
   * @return Whether the block was changed
   */
  bool runOnMachineBasicBlock(MachineBasicBlock &MBB);

  /**
   * Check whether the register for a branch is guaranteed to be zero
   *
   * This function should be called on the outputs of `analyzeBranch`. That call
   * should've been able to decode the branch, and it should've flagged this as
   * a conditional branch. Furthermore, `MBB` must be one of the potential
   * successors of the block containing the analyzed branch.
   *
   * @param[in] MBB The block to check for a zeroed register
   * @param[in] Cond The nonempty output of `analyzeBranch`
   * @param[in] TBB Where the predecessor branches on true, from `analyzeBranch`
   * @return Whether the source register for the corresponding branch is zero in
   * `MBB`
   */
  bool guaranteesZeroRegInBlock(MachineBasicBlock &MBB,
                                const SmallVectorImpl<MachineOperand> &Cond,
                                MachineBasicBlock *TBB);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_PASS_LC32REDUNDANTCLEARELIMNINATION_H
