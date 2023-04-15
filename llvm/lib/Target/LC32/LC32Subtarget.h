//===-- LC32Subtarget.h - Define Subtarget for the LC-3.2 ------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module contains references to all the other parts of code generation for
// the LC-3.2. It also contains information about the target variants, but we
// only have the one.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32SUBTARGET_H
#define LLVM_LIB_TARGET_LC32_LC32SUBTARGET_H

#include "llvm/CodeGen/TargetSubtargetInfo.h"

#define GET_SUBTARGETINFO_HEADER
#include "LC32GenSubtargetInfo.inc"

namespace llvm {

class LC32Subtarget : public LC32GenSubtargetInfo {
public:
  LC32Subtarget(const Triple &TT, const std::string &CPU, const std::string &FS,
                const TargetMachine &TM);
  void ParseSubtargetFeatures(StringRef CPU, StringRef TuneCPU, StringRef FS);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32SUBTARGET_H
