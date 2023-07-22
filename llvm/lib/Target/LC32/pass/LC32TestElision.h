//==- LC32TestElision.h - Test Elision Pass for LC-3.2 ----------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Until this point, conditional branches have been using the `C_BR_CMP_ZERO`
// instruction. The `LC32AsmPrinter` class expands those into two instructions:
// one to test the register, and one to do the actual branch. However, the test
// instruction is redundant if the preceeding instructions already set up the
// condition codes appropriately.
//
// Therefore, this pass scans each basic block for opportunities to replace
// `C_BR_CMP_ZERO` instructions with regular `BR` instructions. This way, we can
// for example turn
// ```
// $r0 = ADDi $r0, 5
// C_BR_CMP_ZERO 2, $r0, %bb
// ```
// into
// ```
// $r0 = ADDi $r0, 5
// BR 2, %bb, implicit $r0
// ```
//
// This pass only looks at instructions within a single basic block. It assumes
// the condition codes are unknown on entry into the block. This can probably be
// optimized, but it's probably not worth it.
//
// This is the only place where instructions and condition codes explicitly
// interact. If that part of the ISA changes, the `transfer` method will likely
// have to be modified.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_PASS_LC32TESTELISION_H
#define LLVM_LIB_TARGET_LC32_PASS_LC32TESTELISION_H

#include "llvm/CodeGen/MachineFunctionPass.h"

namespace llvm {

// Required for INITIALIZE_PASS
void initializeLC32TestElisionPass(PassRegistry &);

class LC32TestElision : public MachineFunctionPass {
public:
  static char ID;
  LC32TestElision() : MachineFunctionPass(this->ID) {
    initializeLC32TestElisionPass(*PassRegistry::getPassRegistry());
  }
  StringRef getPassName() const override { return "LC-3.2 Test Elision"; }

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
   * Transfer function for the condition codes
   *
   * @param[in] MI The next instruction to be executed
   * @param[in] CC What register is held in the condition codes before `MI`
   * @return The register that will be held in the condition codes after `MI`
   */
  Register transfer(const MachineInstr &MI, Register CC);
  /**
   * Update instruction given the current conditon codes
   *
   * The new instruction sequence should be identical to the old one, including
   * in terms of effects on the condition codes. It can output a sequence of
   * instructions for one input instruction, though that is currently not done.
   *
   * The `MI` argument is a reference to an instruction inside a basic block. If
   * it is to be modified, the instruction should be deleted, with the
   * replacement in its place.
   *
   * @param[inout] MI The instruction to modify
   * @param[in] CC What register is held in the condition codes before the
   * instruction pointed to by `MBBI`
   * @return Whether a change was made
   */
  bool update(MachineInstr &MI, Register CC);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_PASS_LC32TESTELISION_H
