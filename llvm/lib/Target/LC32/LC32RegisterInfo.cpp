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
#include "llvm/CodeGen/MachineFunction.h"
using namespace llvm;
#define DEBUG_TYPE "LC32RegisterInfo"

#define GET_REGINFO_TARGET_DESC
#include "LC32GenRegisterInfo.inc"

LC32RegisterInfo::LC32RegisterInfo() : LC32GenRegisterInfo(LC32::LR) {}

const MCPhysReg *
LC32RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  // All registers get saved in the prologue, so we don't have to deal with this
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
  llvm_unreachable("UNIMPLEMENTED");
}
