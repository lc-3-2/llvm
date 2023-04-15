//===-- LC32Subtarget.cpp - Define Subtarget for the LC-3.2 ----*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32Subtarget.h"
using namespace llvm;
#define DEBUG_TYPE "LC32Subtarget"

// Provides: LC32GenSubtargetInfo
// Provides: ParseSubtargetFeatures
#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "LC32GenSubtargetInfo.inc"

LC32Subtarget::LC32Subtarget(const Triple &TT, const std::string &CPU,
                             const std::string &FS, const TargetMachine &TM)
    : LC32GenSubtargetInfo(TT, CPU, CPU, FS), TLInfo(TM, *this) {}

const LC32TargetLowering *LC32Subtarget::getTargetLowering() const {
  return &this->TLInfo;
}

const LC32FrameLowering *LC32Subtarget::getFrameLowering() const {
  return &this->FrameLowering;
}
