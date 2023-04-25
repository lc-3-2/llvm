//==- LC32FrameLowering.cpp - Define Frame Lowering for LC-3.2 --*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32FrameLowering.h"
#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
using namespace llvm;
#define DEBUG_TYPE "LC32FrameLowering"

LC32FrameLowering::LC32FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0,
                          Align(4)) {}

void LC32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  // Populate variables
  assert(&MF.front() == &MBB && "Shrink-wrapping not supported");
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI->getDebugLoc();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Compute the offset for the frame pointer
  // Note that this does not affect the offsets of the frame indicies
  MFI.setOffsetAdjustment(-MFI.getStackSize() - 16);

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Save LR and FP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::STW))
      .addReg(LC32::LR, RegState::Kill)
      .addReg(LC32::SP)
      .addImm(-8);
  BuildMI(MBB, MBBI, dl, TII.get(LC32::STW))
      .addReg(LC32::FP, RegState::Kill)
      .addReg(LC32::SP)
      .addImm(-12);

  // Compute FP
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
          .addReg(LC32::FP, RegState::Define)
          .addReg(LC32::SP, RegState::Kill)
          .addImm(-16);
  n->getOperand(3).setIsDead();

  // Build the stack pointer
  if (MFI.getStackSize() == 0) {
    // Case where we have no local variables
    n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
            .addReg(LC32::SP, RegState::Define)
            .addReg(LC32::FP)
            .addImm(0);
    n->getOperand(3).setIsDead();

  } else {
    // We have local variables, so we need to offset the stack
    int64_t to_go = -MFI.getStackSize();
    assert(to_go < 0);

    // If we only have a small amount, do repeated adds
    // Threshold was chosen to be the size of LOADIMMH
    if (to_go >= -64) {
      // Keep track of if this is the first iteration
      bool first = true;
      // Do the adds
      while (to_go < 0) {
        int64_t to_add = std::max(-16l, to_go);
        n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
                .addReg(LC32::SP, RegState::Define)
                .addReg(first ? LC32::FP : LC32::SP, first ? 0 : RegState::Kill)
                .addImm(to_add);
        n->getOperand(3).setIsDead();
        // Next iteration
        assert(to_go - to_add > to_go);
        to_go -= to_add;
        first = false;
      }
      // Done
      assert(to_go == 0);
      return;
    }

    llvm_unreachable("TODO");
  }
}

void LC32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  // Populate variables
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Restore SP
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
          .addReg(LC32::SP, RegState::Define)
          .addReg(LC32::FP, RegState::Kill)
          .addImm(12);
  n->getOperand(3).setIsDead();

  // Restore LR and FP
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW))
          .addReg(LC32::LR, RegState::Define)
          .addReg(LC32::SP)
          .addImm(-4);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW))
          .addReg(LC32::FP, RegState::Define)
          .addReg(LC32::SP)
          .addImm(-8);
  n->getOperand(3).setIsDead();
}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }
