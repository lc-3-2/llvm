//===-- LC32RegisterInfo.h - LC-3.2 Register Information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module contains information about the registers for the LC-3.2, and
// specifically how they relate to code generation. It also contains the
// `eliminateFrameIndex` method.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32REGISTERINFO_H
#define LLVM_LIB_TARGET_LC32_LC32REGISTERINFO_H

#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "LC32GenRegisterInfo.inc"

namespace llvm {
class LC32InstrInfo;

class LC32RegisterInfo : public LC32GenRegisterInfo {
public:
  LC32RegisterInfo();

  // Code generation
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  // For debug information
  Register getFrameRegister(const MachineFunction &MF) const override;

  // We don't support stack realignment
  // This is checked when determining where to place the emergency spill slots.
  // We want them near the FP.
  bool canRealignStack(const MachineFunction &MF) const override;
  // We need the scavenger for frame indices
  bool requiresRegisterScavenging(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;

  /**
   * Generate instructions to add a large constant to a register
   *
   * This is a very common task, so we factor out the code into a function. It
   * generates instructions to add a large constant to a source register and put
   * the result in a destination register. It either generates repeated
   * additions, or it uses the AT as a staging area.
   *
   * Even though this is logically private, it has to be public since
   * LC32FrameLowering uses it.
   *
   * @param [in] dr The register to add into
   * @param [in] sr The register to add from
   * @param [in] imm The value to add
   * @param [in] alias Allow sr and dr to alias, at the expense of using AT
   * @param [in] dr_flags The liveliness state of the destination register once
   *  the code is done. Should be either `Define` or `Dead`.
   * @param [in] sr_flags The liveliness state of the source register
   * @see MaxRepeatedAdd
   */
  void genAddLargeImm(const LC32InstrInfo &TII, MachineBasicBlock &MBB,
                      MachineBasicBlock::iterator MBBI, DebugLoc &dl,
                      Register dr, Register sr, int64_t imm, bool alias = false,
                      unsigned dr_flags = RegState::Define,
                      unsigned sr_flags = 0u) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32REGISTERINFO_H
