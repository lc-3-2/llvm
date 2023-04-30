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
#include "llvm/Support/ErrorHandling.h"
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
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

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
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::FP)
          .addReg(LC32::SP, RegState::Kill)
          .addImm(-16);
  n->getOperand(3).setIsDead();

  // Build the stack pointer
  TRI.genAddLargeImm(TII, MBB, MBBI, dl, LC32::SP, LC32::FP,
                     -MFI.getStackSize());
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
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::SP)
          .addReg(LC32::FP, RegState::Kill)
          .addImm(12);
  n->getOperand(3).setIsDead();

  // Restore LR and FP
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW), LC32::LR)
          .addReg(LC32::SP)
          .addImm(-4);
  n->getOperand(3).setIsDead();
  n = BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW), LC32::FP)
          .addReg(LC32::SP)
          .addImm(-8);
  n->getOperand(3).setIsDead();
}

MachineBasicBlock::iterator LC32FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  // Populate variables
  DebugLoc dl = MI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

  // Get the size associated with this pseudo
  uint64_t amt = TII.getFrameSize(*MI);
  // Check if we actually need to do anything
  if (amt != 0) {

    // Align to four bytes
    assert(this->getStackAlign() == 4 &&
           "LC-3.2 call stack should be word aligned");
    amt = alignTo(amt, this->getStackAlign());

    // Do the addition
    if (MI->getOpcode() == TII.getCallFrameSetupOpcode())
      TRI.genAddLargeImm(TII, MBB, MI, dl, LC32::SP, LC32::SP, -amt);
    else if (MI->getOpcode() == TII.getCallFrameDestroyOpcode())
      TRI.genAddLargeImm(TII, MBB, MI, dl, LC32::SP, LC32::SP, amt + 4);
    else
      llvm_unreachable("Tried to eliminate bad instruction");
  }

  // Remember to erase the original pseudo
  return MBB.erase(MI);
}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }
