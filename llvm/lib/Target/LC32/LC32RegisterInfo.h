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

#include "llvm/CodeGen/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "LC32GenRegisterInfo.inc"

namespace llvm {

class LC32RegisterInfo : public LC32GenRegisterInfo {
public:
  LC32RegisterInfo();

  // Code generation
  const MCPhysReg *getCalleeSavedRegs(const MachineFunction *MF) const override;
  BitVector getReservedRegs(const MachineFunction &MF) const override;

  // For debug information
  Register getFrameRegister(const MachineFunction &MF) const override;

  bool eliminateFrameIndex(MachineBasicBlock::iterator MI, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32REGISTERINFO_H
