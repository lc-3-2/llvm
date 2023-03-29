//===-- LC32MCTargetDesc.cpp - LC-3.2 Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

// #define GET_REGINFO_MC_DESC
// #include "LC32GenRegisterInfo.inc"

// #define GET_INSTRINFO_MC_DESC
// #define ENABLE_INSTR_PREDICATE_VERIFIER
// #include "LC32GenInstrInfo.inc"

// #define GET_SUBTARGETINFO_MC_DESC
// #include "LC32GenSubtargetInfo.inc"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32TargetMC() {
  // Auto-discovered method
  // Associate all the components of the MC layer with the LC-3.2 target handle
  Target &T = getTheLC32Target();
}
