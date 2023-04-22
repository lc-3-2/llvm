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

  // Populate variables
  MachineBasicBlock &MBB = *MI->getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc dl = MI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  int FrameIndex = MI->getOperand(FIOperandNum).getIndex();
  int Offset = MF.getFrameInfo().getObjectOffset(FrameIndex);

  // Handle loads and stores
  if (MI->getOpcode() == LC32::LDW || MI->getOpcode() == LC32::STW) {

    // Get the offset from the instruction and incorporate
    Offset += MI->getOperand(FIOperandNum + 1).getImm();

    // Check whether we can materialize the offset inside the operation
    if (isShiftedInt<6, 2>(Offset)) {
      MI->getOperand(FIOperandNum).ChangeToRegister(LC32::FP, false);
      MI->getOperand(FIOperandNum + 1).ChangeToImmediate(Offset, false);
      return false;
    }

    // Otherwise, use AT as the index register
    // TODO: Use a more intelligent method. Right now this just does repeated
    // addition.
    assert((Offset < -128 || Offset > 124) && "Bad check for bounds");
    {
      // Do the additions
      ssize_t to_go = Offset;
      bool first = true;
      while (to_go < -256 || to_go > 252) {
        ssize_t to_off = to_go > 0 ? 16 : 16;
        MachineInstr *n = BuildMI(MBB, MI, dl, TII.get(LC32::ADDi))
                              .addReg(LC32::AT)
                              .addReg(first ? LC32::FP : LC32::AT)
                              .addImm(to_off);
        n->getOperand(3).setIsDead();
        to_go += to_off;
        first = false;
      }
      // Take the final step
      MI->getOperand(FIOperandNum).ChangeToRegister(LC32::FP, false);
      MI->getOperand(FIOperandNum + 1).ChangeToImmediate(to_go, false);
      return false;
    }
  }

  llvm_unreachable("Bad instruction with frame index");
}
