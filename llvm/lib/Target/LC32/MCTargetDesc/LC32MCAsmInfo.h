//===-- LC32MCAsmInfo.h - LC-3.2 Asm Properties ----------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module provides information for reading and writing assembly files. It
// has a lot of configuration options to specify what exactly the assembly
// should look like. Those options are mostly initialized by the constructor.
//
// See: llvm/MC/MCAsmInfo.h
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMINFO_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMINFO_H

#include "llvm/MC/MCAsmInfoELF.h"

namespace llvm {
class Triple;

class LC32MCAsmInfo : public MCAsmInfoELF {
public:
  LC32MCAsmInfo(const Triple &TT, const MCTargetOptions &Options);
  bool isValidUnquotedName(StringRef Name) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMINFO_H
