//==- LC32FrameLowering.cpp - Define Frame Lowering for LC-3.2 --*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32FrameLowering.h"
#include "LC32InstrInfo.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;
#define DEBUG_TYPE "LC32FrameLowering"

LC32FrameLowering::LC32FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), 0,
                          Align(4), false) {}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void LC32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  // Populate variables
  // If the first block is empty, the debug location will be invalid. Deal with
  // that
  assert(&MF.front() == &MBB && "Shrink-wrapping not supported");
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

  // Compute the offset for the frame pointer
  // Note that this does not affect the offsets of the frame indicies
  MFI.setOffsetAdjustment(-MFI.getStackSize() - 16);

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
  BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::FP)
      .addReg(LC32::SP, RegState::Kill)
      .addImm(-16);

  // Build the stack pointer
  TRI.genAddLargeImm(MBBI, dl, LC32::SP, LC32::FP, -MFI.getStackSize());
}

void LC32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  // Populate variables
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Restore SP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::SP)
      .addReg(LC32::FP, RegState::Kill)
      .addImm(12);

  // Restore LR and FP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW), LC32::LR)
      .addReg(LC32::SP)
      .addImm(-4);
  BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW), LC32::FP)
      .addReg(LC32::SP)
      .addImm(-8);
}

MachineBasicBlock::iterator LC32FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  // Populate variables
  DebugLoc dl = MI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

  // Check the type of the variable we're eliminating
  assert(TII.getCallFrameSetupOpcode() != TII.getCallFrameDestroyOpcode() &&
         "Call frame setup and destroy are same");
  assert((MI->getOpcode() == TII.getCallFrameSetupOpcode() ||
          MI->getOpcode() == TII.getCallFrameDestroyOpcode()) &&
         "Bad type of instruction to eliminate");

  // Get the size associated with this pseudo
  uint64_t amt = TII.getFrameSize(*MI);
  // Align to four bytes
  assert(this->getStackAlign() == 4 && "LC-3.2 stack should be word aligned");
  amt = alignTo(amt, this->getStackAlign());

  // If the amount is zero, we don't have to do anything
  // The amt above should include the return value if we need to pop it
  if (amt != 0) {
    // Handle setup and teardown
    if (MI->getOpcode() == TII.getCallFrameSetupOpcode()) {
      TRI.genAddLargeImm(MI, dl, LC32::SP, LC32::SP, -amt, true,
                         RegState::Define, RegState::Kill);
    } else if (MI->getOpcode() == TII.getCallFrameDestroyOpcode()) {
      TRI.genAddLargeImm(MI, dl, LC32::SP, LC32::SP, amt, true,
                         RegState::Define, RegState::Kill);
    } else {
      llvm_unreachable("Tried to eliminate bad instruction");
    }
  }

  // Remember to erase the original pseudo
  return MBB.erase(MI);
}

void LC32FrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  // Check that we actually got a register scavenger
  assert(RS != nullptr && "Must have a register scavenger");
  // Populate variables
  const LC32RegisterInfo *RI =
      MF.getSubtarget<LC32Subtarget>().getRegisterInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Count the number of scavenging slots we need:
  // 1. if the stack frame is too large. This is an underestimate, so compensate
  // 2. if branches can be out of range
  unsigned num_scav = 0;
  if (!isInt<6 - 1>(MFI.estimateStackSize(MF)))
    num_scav++;
  // FIXME: Implement 2

  // Create the scavenging indicies
  for (unsigned i = 0; i < num_scav; i++) {
    RS->addScavengingFrameIndex(
        MFI.CreateStackObject(RI->getSpillSize(LC32::GPRRegClass),
                              RI->getSpillAlign(LC32::GPRRegClass), false));
  }
}
