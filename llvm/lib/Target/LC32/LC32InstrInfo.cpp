//===-- LC32InstrInfo.cpp - LC-3.2 Instruction Information ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "LC32Subtarget.h"
using namespace llvm;
#define DEBUG_TYPE "LC32InstrInfo"

#define GET_INSTRINFO_CTOR_DTOR
#include "LC32GenInstrInfo.inc"

LC32InstrInfo::LC32InstrInfo(LC32Subtarget &STI) {}

const LC32RegisterInfo &LC32InstrInfo::getRegisterInfo() const {
  return this->RegisterInfo;
}
