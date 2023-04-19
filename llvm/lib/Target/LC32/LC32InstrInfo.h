//===-- LC32InstrInfo.h - LC-3.2 Instruction Information --------*- C++ -*-===//
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

#ifndef LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H
#define LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H

#include "LC32RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "LC32GenInstrInfo.inc"

namespace llvm {
class LC32Subtarget;

class LC32InstrInfo : public LC32GenInstrInfo {
public:
  explicit LC32InstrInfo(LC32Subtarget &STI);
  const LC32RegisterInfo &getRegisterInfo() const;

private:
  // Modules need the register information to work with this class. Therefore,
  // we provide access to it.
  // See: MSP430InstrInfo.h
  LC32RegisterInfo RegisterInfo;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H
