//===-- LC32RegisterInfo.cpp - LC-3.2 Register Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32RegisterInfo.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
using namespace llvm;
#define DEBUG_TYPE "LC32RegisterInfo"

#define GET_REGINFO_TARGET_DESC
#include "LC32GenRegisterInfo.inc"

LC32RegisterInfo::LC32RegisterInfo() : LC32GenRegisterInfo(LC32::LR) {}

const MCPhysReg *
LC32RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  // We use caller save
  static const MCPhysReg CALLEE_SAVED_REGS[] = {0};
  return CALLEE_SAVED_REGS;
}

BitVector LC32RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector ret(this->getNumRegs());
  ret.set(LC32::AT);
  ret.set(LC32::GP);
  ret.set(LC32::FP);
  ret.set(LC32::SP);
  ret.set(LC32::LR);
  return ret;
}

Register LC32RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return LC32::FP;
}

bool LC32RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {

  // Populate variables
  // Note that offsets are truncated to the size of memory
  MachineBasicBlock &MBB = *MI->getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc dl = MI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  int FrameIndex = MI->getOperand(FIOperandNum).getIndex();
  int32_t Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  // Use this to set condition codes as dead, which they should be
  MachineInstr *n = nullptr;

  // Handle loads and stores
  if (MI->getOpcode() == LC32::LDB || MI->getOpcode() == LC32::STB ||
      MI->getOpcode() == LC32::LDH || MI->getOpcode() == LC32::STH ||
      MI->getOpcode() == LC32::LDW || MI->getOpcode() == LC32::STW) {
    assert(FIOperandNum == 1 && "Bad frame index operand index");

    // Add the operand to the offset
    Offset += MI->getOperand(2).getImm();

    // Check if the offset is in range
    bool in_range = false;
    if (MI->getOpcode() == LC32::LDB || MI->getOpcode() == LC32::STB)
      in_range = isShiftedInt<6, 0>(Offset);
    else if (MI->getOpcode() == LC32::LDH || MI->getOpcode() == LC32::STH)
      in_range = isShiftedInt<6, 1>(Offset);
    else if (MI->getOpcode() == LC32::LDW || MI->getOpcode() == LC32::STW)
      in_range = isShiftedInt<6, 2>(Offset);

    // If the offset is in range, then we're good to just use the FP
    if (in_range) {
      MI->getOperand(1).ChangeToRegister(LC32::FP, false);
      MI->getOperand(2).ChangeToImmediate(Offset);
      return false;
    }

    // Use the smallest instruction that fits
    auto instr_to_use =
        isInt<16>(Offset) ? LC32::P_LOADCONSTH : LC32::P_LOADCONSTW;
    // Use AT as a staging area in which to compute the address
    n = BuildMI(MBB, MI, dl, TII.get(instr_to_use), LC32::AT).addImm(Offset);
    n->getOperand(2).setIsDead();
    n = BuildMI(MBB, MI, dl, TII.get(LC32::ADDr), LC32::AT)
            .addReg(LC32::FP)
            .addReg(LC32::AT, RegState::Kill);
    n->getOperand(3).setIsDead();
    MI->getOperand(1).ChangeToRegister(LC32::AT, false, false, true);
    MI->getOperand(2).ChangeToImmediate(0);
    return false;
  }

  // Handle C_LEA_FRAMEINDEX
  // This erases it, so we don't have to lower it later
  if (MI->getOpcode() == LC32::C_LEA_FRAMEINDEX) {
    assert(FIOperandNum == 1 && "Bad frame index operand index");

    // If the offset is small enough, we can just ADD
    if (isInt<5>(Offset)) {
      n = BuildMI(MBB, MI, dl, TII.get(LC32::ADDi))
              .addReg(MI->getOperand(0).getReg(),
                      getRegState(MI->getOperand(0)))
              .addReg(LC32::FP)
              .addImm(Offset);
      n->getOperand(3).setIsDead();
      MBB.erase(MI);
      return false;
    }

    // Use the smallest instruction that fits
    auto instr_to_use =
        isInt<16>(Offset) ? LC32::P_LOADCONSTH : LC32::P_LOADCONSTW;
    // Put the offset into AT and ADD from there
    n = BuildMI(MBB, MI, dl, TII.get(instr_to_use), LC32::AT).addImm(Offset);
    n->getOperand(2).setIsDead();
    n = BuildMI(MBB, MI, dl, TII.get(LC32::ADDr))
            .addReg(MI->getOperand(0).getReg(), getRegState(MI->getOperand(0)))
            .addReg(LC32::FP)
            .addReg(LC32::AT, RegState::Kill);
    n->getOperand(3).setIsDead();
    MBB.erase(MI);
    return false;
  }

  llvm_unreachable("Bad instruction with frame index");
}
