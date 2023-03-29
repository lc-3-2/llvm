//===-- LC32TargetInfo.cpp - LC-3.2 Target Implementation -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

Target &llvm::getTheLC32Target() {
  // The target's handle is just a reference to a `Target` object
  static Target TheLC3bTarget;
  return TheLC3bTarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32TargetInfo() {
  // Auto-discovered method
  // Register LC-3.2 target handle
  RegisterTarget<Triple::lc_3_2> X(
    getTheLC32Target(),
    "lc-3.2", "LC-3.2 ISA", "LC-3.2"
  );
}
