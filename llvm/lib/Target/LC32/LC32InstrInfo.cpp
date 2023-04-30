//===-- LC32InstrInfo.cpp - LC-3.2 Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
using namespace llvm;
#define DEBUG_TYPE "LC32InstrInfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "LC32GenInstrInfo.inc"

LC32InstrInfo::LC32InstrInfo(LC32Subtarget &STI)
    : LC32GenInstrInfo(LC32::C_ADJCALLSTACKUP, LC32::C_ADJCALLSTACKDOWN, ~0u,
                       LC32::C_RET) {}

const LC32RegisterInfo &LC32InstrInfo::getRegisterInfo() const {
  return this->RegisterInfo;
}

void LC32InstrInfo::storeRegToStackSlot(
    MachineBasicBlock &MBB, MachineBasicBlock::iterator MI, Register SrcReg,
    bool isKill, int FrameIndex, const TargetRegisterClass *RC,
    const TargetRegisterInfo *TRI, Register VReg) const {
  // See: LanaiInstrInfo.cpp

  // Check that the register class is what we expect
  assert(LC32::GPRRegClass.hasSubClassEq(RC) && "Bad register class to store");

  // Get the debug location
  DebugLoc dl;
  if (MI != MBB.end())
    dl = MI->getDebugLoc();

  // Do the store
  // Remember, frame indices are lowered in LC32RegisterInfo.cpp
  BuildMI(MBB, MI, dl, this->get(LC32::STW))
      .addReg(SrcReg, getKillRegState(isKill))
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void LC32InstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                         MachineBasicBlock::iterator MI,
                                         Register DestReg, int FrameIndex,
                                         const TargetRegisterClass *RC,
                                         const TargetRegisterInfo *TRI,
                                         Register VReg) const {
  // See: LanaiInstrInfo.cpp

  // Check that the register class is what we expect
  assert(LC32::GPRRegClass.hasSubClassEq(RC) && "Bad register class to store");

  // Get the debug location
  DebugLoc dl;
  if (MI != MBB.end())
    dl = MI->getDebugLoc();

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Do the store
  // Remember, frame indices are lowered in LC32RegisterInfo.cpp
  n = BuildMI(MBB, MI, dl, this->get(LC32::LDW), DestReg)
          .addFrameIndex(FrameIndex)
          .addImm(0);
  n->getOperand(3).setIsDead();
}
