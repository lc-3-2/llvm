//===-- LC32RegisterInfo.cpp - LC-3.2 Register Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32RegisterInfo.h"
#include "LC32CLOpts.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/MathExtras.h"
#include <assert.h>
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32RegisterInfo"

#define GET_REGINFO_TARGET_DESC
#include "LC32GenRegisterInfo.inc"

LC32RegisterInfo::LC32RegisterInfo() : LC32GenRegisterInfo(LC32::LR) {}

const MCPhysReg *
LC32RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  // Almost everything is caller-save
  // The only exception is the global pointer if we're using it as a
  // general-purpose register. In that case, the caller might be expecting it to
  // remain unchanged.
  static const MCPhysReg CALLEE_SAVED_REGS[] = {LC32::GP, 0};
  return CALLEE_SAVED_REGS;
}

BitVector LC32RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector ret(this->getNumRegs());

  // Frame pointer and stack pointer have to be reserved
  // This is because we don't have frame pointer elimination
  ret.set(LC32::FP);
  ret.set(LC32::SP);

  // Set global pointer and link register
  if (!UseR4)
    ret.set(LC32::GP);
  if (!UseR7)
    ret.set(LC32::LR);

  return ret;
}

Register LC32RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return LC32::FP;
}

bool LC32RegisterInfo::canRealignStack(const MachineFunction &MF) const {
  return false;
}

bool LC32RegisterInfo::requiresRegisterScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool LC32RegisterInfo::requiresFrameIndexScavenging(
    const MachineFunction &MF) const {
  return true;
}

bool LC32RegisterInfo::supportsBackwardScavenger() const { return true; }

bool LC32RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator MI,
                                           int SPAdj, unsigned FIOperandNum,
                                           RegScavenger *RS) const {
  // Populate variables
  // Note that offsets are truncated to the size of memory
  MachineBasicBlock &MBB = *MI->getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const TargetFrameLowering *TFI = MF.getSubtarget().getFrameLowering();
  DebugLoc dl = MI->getDebugLoc();
  int FI = MI->getOperand(FIOperandNum).getIndex();
  assert(SPAdj == 0 && "We don't use SPAdj");

  // Compute the offset for the frame index
  // Remember, these offsets are relative to the stack pointer on function
  // entry. Therefore, we should correct by adding 16. We actually get another
  // function to do the computation for us, and check it gives the right result.
  Register Base;
  StackOffset StackOffset = TFI->getFrameIndexReference(MF, FI, Base);
  assert(Base == LC32::FP && "R5 should be used to access stack slots");
  assert(StackOffset.getScalable() == 0 && "Unexpected value for the offset");
  assert(StackOffset.getFixed() == MF.getFrameInfo().getObjectOffset(FI) + 16 &&
         "Unexpected value for the offset");
  int32_t Offset = StackOffset.getFixed();

  // Handle loads and stores
  if (MI->getOpcode() == LC32::LDB || MI->getOpcode() == LC32::STB ||
      MI->getOpcode() == LC32::LDH || MI->getOpcode() == LC32::STH ||
      MI->getOpcode() == LC32::LDW || MI->getOpcode() == LC32::STW) {
    assert(FIOperandNum == 1 && "Bad frame index operand index");

    // Add the operand to the offset
    Offset += MI->getOperand(2).getImm();

    // Check if the offset is in range
    // Also keep track of the "word size" of the instruction in bits
    bool in_range = false;
    size_t word_size = ~0ull;
    switch (MI->getOpcode()) {
    case LC32::LDB:
    case LC32::STB:
      in_range = isShiftedInt<6, 0>(Offset);
      word_size = 1;
      break;
    case LC32::LDH:
    case LC32::STH:
      in_range = isShiftedInt<6, 1>(Offset);
      word_size = 2;
      break;
    case LC32::LDW:
    case LC32::STW:
      in_range = isShiftedInt<6, 2>(Offset);
      word_size = 4;
      break;
    default:
      llvm_unreachable("Not all cases handled");
    }
    assert((word_size == 1 || word_size == 2 || word_size == 4) &&
           "Did not set word_size correctly");

    // If the offset is in range, then we're good to just use the FP
    if (in_range) {
      MI->getOperand(1).ChangeToRegister(LC32::FP, false);
      MI->getOperand(2).ChangeToImmediate(Offset);
      return false;
    }

    // Figure out what temporary register to use to store the address
    Register tr;
    switch (MI->getOpcode()) {
    case LC32::LDB:
    case LC32::LDH:
    case LC32::LDW:
      // For loads, we can just reuse the destination register
      tr = MI->getOperand(0).getReg();
      break;
    case LC32::STB:
    case LC32::STH:
    case LC32::STW:
      // For stores, use a virtual register
      // PEI will eliminate this since we told it too
      // See: llvm/lib/CodeGen/PrologEpilogInserter.cpp:281
      tr = MRI.createVirtualRegister(&LC32::GPRRegClass);
      break;
    default:
      llvm_unreachable("Not all cases handled");
    }

    // Compute how much of the offset to fold into the instruction itself. We
    // want to do as much work as possible with this, so that the added
    // instructions have to do less work.
    int32_t FoldedOffset;
    {
      // Compute the largest multiple that fits. We assume that negative numbers
      // round toward zero.
      static_assert(-1 / 2 == 0, "Negative division should round to zero");
      int32_t fo_w = Offset / word_size;
      // Restrict to 6-bit signed
      fo_w = std::min(std::max(fo_w, -32), 31);
      assert(isInt<6>(fo_w) && "Did not restrict to 6-bit signed");

      // Convert to bytes
      FoldedOffset = fo_w * word_size;
    }

    // Construct the address and load from it
    this->genAddLargeImm(MI, dl, tr, LC32::FP, Offset - FoldedOffset);
    MI->getOperand(1).ChangeToRegister(tr, false, false, true);
    MI->getOperand(2).ChangeToImmediate(FoldedOffset);
    return false;
  }

  // Handle C_LEA_FRAMEINDEX
  // This erases it, so we don't have to lower it later
  if (MI->getOpcode() == LC32::C_LEA_FRAMEINDEX) {
    assert(FIOperandNum == 1 && "Bad frame index operand index");

    this->genAddLargeImm(MI, dl, MI->getOperand(0).getReg(), LC32::FP, Offset,
                         false, getRegState(MI->getOperand(0)));
    MBB.erase(MI);
    return false;
  }

  llvm_unreachable("Bad instruction with frame index");
}

void LC32RegisterInfo::genAddLargeImm(MachineBasicBlock::iterator MBBI,
                                      DebugLoc &dl, Register dr, Register sr,
                                      int64_t imm, bool alias,
                                      unsigned dr_flags,
                                      unsigned sr_flags) const {

  // Populate variables
  MachineBasicBlock &MBB = *MBBI->getParent();
  MachineFunction &MF = *MBB.getParent();
  MachineRegisterInfo &MRI = MF.getRegInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Check aliasing
  if (!alias)
    assert(sr != dr && "Source and destination may not alias");

  // If the immediate is zero, just ADDi
  if (imm == 0) {
    // Only emit if the source is not the same as the destination. In that
    // case, this instruction would be a NOP
    if (sr != dr)
      BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
          .addReg(dr, dr_flags)
          .addReg(sr, sr_flags)
          .addImm(0);
    return;
  }

  // Compute if we can get away with the repeated adds
  if (imm >= INT64_C(-16) * static_cast<int64_t>(MaxRepeatedOps) &&
      imm <= INT64_C(15) * static_cast<int64_t>(MaxRepeatedOps)) {
    int64_t to_go = imm;
    bool first_loop = true;
    while (to_go != 0) {
      int64_t to_add = std::max(INT64_C(-16), std::min(INT64_C(15), to_go));
      bool last_loop = to_add == to_go;

      BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi))
          .addReg(dr, last_loop ? dr_flags
                                : static_cast<unsigned>(RegState::Define))
          .addReg(first_loop ? sr : dr,
                  first_loop ? sr_flags : static_cast<unsigned>(RegState::Kill))
          .addImm(to_add);

      to_go -= to_add;
      first_loop = false;
    }
    return;
  }

  // Otherwise, use a staging register
  // Start by calculating which instruction to use
  auto instr = isInt<16>(imm) ? LC32::P_LOADCONSTH : LC32::P_LOADCONSTW;
  // Load the offset into either dr or a vreg, depending on whether the operands
  // alias
  Register tr = alias ? MRI.createVirtualRegister(&LC32::GPRRegClass) : dr;
  BuildMI(MBB, MBBI, dl, TII.get(instr), tr).addImm(static_cast<int32_t>(imm));
  // Do the add
  BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDr))
      .addReg(dr, dr_flags)
      .addReg(sr, sr_flags)
      .addReg(tr, RegState::Kill);
  // Done
  return;
}
