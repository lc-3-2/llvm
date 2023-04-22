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
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), -12,
                          Align(4)) {}

void LC32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  // Populate variables
  assert(&MF.front() == &MBB && "Shrink-wrapping not supported");
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc DL = MBBI->getDebugLoc();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Compute the offset for the frame pointer
  MFI.setOffsetAdjustment(-MFI.getStackSize() - 20);

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Save LR and FP
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::LR)
      .addReg(LC32::SP)
      .addImm(-8);
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::FP)
      .addReg(LC32::SP)
      .addImm(-12);

  // Add to SP
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::ADDi))
      .addReg(LC32::FP)
      .addReg(LC32::SP)
      .addImm(-16);
  n->getOperand(3).setIsDead();

  // Save temporary registers
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::R0)
      .addReg(LC32::FP)
      .addImm(-16);
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::R1)
      .addReg(LC32::FP)
      .addImm(-12);
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::R2)
      .addReg(LC32::FP)
      .addImm(-8);
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::AT)
      .addReg(LC32::FP)
      .addImm(-4);
  BuildMI(MBB, MBBI, DL, TII.get(LC32::STW))
      .addReg(LC32::GP)
      .addReg(LC32::FP)
      .addImm(0);

  // Build the stack pointer
  // TODO: Use a more intelligent method. Right now this just does repeated
  // addition.
  {
    size_t to_go = MFI.getStackSize() + 16;
    bool first = true;
    while (to_go > 0) {
      size_t to_off = to_go % 16 == 0 ? 16 : to_go % 16;
      n = BuildMI(MBB, MBBI, DL, TII.get(LC32::ADDi))
          .addReg(LC32::SP)
          .addReg(first ? LC32::FP : LC32::SP)
          .addImm(-to_off);
      n->getOperand(3).setIsDead();
      to_go -= to_off;
      first = false;
    }
  }
}

void LC32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  // Populate variables
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc DL = MBBI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Restore temporary registers
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::R0)
      .addReg(LC32::FP)
      .addImm(-16);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::R1)
      .addReg(LC32::FP)
      .addImm(-12);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::R2)
      .addReg(LC32::FP)
      .addImm(-8);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::AT)
      .addReg(LC32::FP)
      .addImm(-4);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::GP)
      .addReg(LC32::FP)
      .addImm(0);
  n->getOperand(3).setIsDead();

  // Restore SP
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::ADDi))
      .addReg(LC32::SP)
      .addReg(LC32::FP)
      .addImm(0);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::ADDi))
      .addReg(LC32::SP)
      .addReg(LC32::SP)
      .addImm(12);
  n->getOperand(3).setIsDead();

  // Restore LR and FP
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::LR)
      .addReg(LC32::SP)
      .addImm(-4);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, DL, TII.get(LC32::LDW))
      .addReg(LC32::FP)
      .addReg(LC32::SP)
      .addImm(-8);
  n->getOperand(3).setIsDead();
}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }
