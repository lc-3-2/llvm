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
// BR 2, %bb, implicit-use $r0
// ```
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
  };
  StringRef getPassName() const override { return "LC-3.2 Test Elision"; }

  bool runOnMachineFunction(MachineFunction &MF) override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_PASS_LC32TESTELISION_H
