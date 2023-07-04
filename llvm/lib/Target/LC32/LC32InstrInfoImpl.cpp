//===-- LC32InstrInfo.cpp - LC-3.2 Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "LC32Subtarget.h"
#include "LC32TargetMachine.h"
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

unsigned LC32InstrInfo::getInstSizeInBytes(const MachineInstr &MI) const {
  // This function is used by other passes to estimate the size of code blocks.
  // It's fine if this is an overestimate, but it should not be an
  // underestimate.
  // See: llvm/lib/CodeGen/BranchRelaxation.cpp
  // See: llvm/lib/CodeGen/IfConversion.cpp
  unsigned retval;

  const MCInstrDesc &d = MI.getDesc();
  unsigned d_op = d.getOpcode();

  // We shouldn't have relaxed instructions at this point
  assert(d_op != LC32::P_FARJSR && "FARJSR generated");
  // Assert that instructions have been eliminated
  // Note that C_LEA_FRAMEINDEX is still allowed at this point
  assert(MI.getOpcode() != LC32::C_SELECT_CMP_ZERO &&
         "C_SELECT_CMP_ZERO not eliminated");
  assert(MI.getOpcode() != LC32::C_ADJCALLSTACKUP &&
         "C_ADJCALLSTACKUP not eliminated");
  assert(MI.getOpcode() != LC32::C_ADJCALLSTACKDOWN &&
         "C_ADJCALLSTACKDOWN not eliminated");

  // Meta instructions produce no output
  if (d.isMetaInstruction()) {
    retval = 0;
    goto ret;
  }

  // Handle inline assembly
  // This estimates every instruction to be the size specified in MCAsmInfo
  if (d_op == TargetOpcode::INLINEASM || d_op == TargetOpcode::INLINEASM_BR) {
    const MachineFunction &MF = *MI.getParent()->getParent();
    retval = this->getInlineAsmLength(MI.getOperand(0).getSymbolName(),
                                      *MF.getTarget().getMCAsmInfo());
    goto ret;
  }

  // Special case for JSR
  // It will be relaxed, so we use the relaxed size as an estimate
  // We don't do this for BR since we have a pass that relaxes branches in the
  // compiler, while JSR is handled in the assembler.
  if (d_op == LC32::JSR) {
    retval = this->get(LC32::P_FARJSR).getSize();
    goto ret;
  }
  // Same for LEA
  if (d_op == LC32::LEA) {
    retval = this->get(LC32::P_LOADCONSTW).getSize();
    goto ret;
  }

  retval = d.getSize();
  goto ret;

ret:
  assert(retval % 2 == 0 && "Instructions should have even length");
  return retval;
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

  // Do the store
  // Remember, frame indices are lowered in LC32RegisterInfo.cpp
  BuildMI(MBB, MI, dl, this->get(LC32::LDW), DestReg)
      .addFrameIndex(FrameIndex)
      .addImm(0);
}

void LC32InstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                MachineBasicBlock::iterator MI,
                                const DebugLoc &DL, MCRegister DestReg,
                                MCRegister SrcReg, bool KillSrc) const {
  // See: LanaiInstrInfo.cpp

  // Check that the register class is what we expect
  assert(LC32::GPRRegClass.contains(DestReg, SrcReg) && "Bad register copy");

  // Do an ADDi
  BuildMI(MBB, MI, DL, this->get(LC32::ADDi), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc))
      .addImm(0);
}
