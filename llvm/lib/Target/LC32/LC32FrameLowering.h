//==- LC32FrameLowering.h - Define Frame Lowering for LC-3.2 ----*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module handles the stack frame. It handles the prologue and epilogue,
// and manages the frame pointer.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32FRAMELOWERING_H
#define LLVM_LIB_TARGET_LC32_LC32FRAMELOWERING_H

#include "llvm/CodeGen/TargetFrameLowering.h"

namespace llvm {

class LC32FrameLowering : public TargetFrameLowering {
public:
  LC32FrameLowering();
  bool hasFP(const MachineFunction &MF) const override;

  void emitPrologue(MachineFunction &MF, MachineBasicBlock &MBB) const override;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const override;

  // Call frame pseudo instructions were declared in the constructor of
  // LC32InstrInfo. They are the C_ADJCALLSTACKUP/DOWN instructions.
  MachineBasicBlock::iterator
  eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI) const override;

  // Handle scavenger setup
  void processFunctionBeforeFrameFinalized(
      MachineFunction &MF, RegScavenger *RS = nullptr) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32FRAMELOWERING_H
