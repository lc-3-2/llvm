//===-- LC32MCObjectFileInfo.h - LC-3.2 Object File Info -------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class seems to contain information for the structure of the ELF file we
// write. We use this class to change the default alignment of the `.text`
// section. It's aligned to four bytes by default, but we want it aligned to
// two. If the section contains four-byte aligned instructions, then its
// alignment will be bumped up. This way, we save some space by default.
//
// See: llvm/lib/Target/RISCV/MCTargetDesc/RISCVMCObjectFileInfo.h
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCOBJECTFILEINFO_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCOBJECTFILEINFO_H

#include "llvm/MC/MCObjectFileInfo.h"

namespace llvm {

class LC32MCObjectFileInfo : public MCObjectFileInfo {
public:
  unsigned getTextSectionAlignment() const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCOBJECTFILEINFO_H
