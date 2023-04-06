//===-- LC32ELFObjectWriter.h - LC-3.2 ELF Writer -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module provides information for translating assembly fixups to
// relocations. The workhorse is the `getRelocType` method.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32ELFOBJECTTARGETWRITER_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32ELFOBJECTTARGETWRITER_H

#include "llvm/MC/MCELFObjectWriter.h"

namespace llvm {

class LC32ELFObjectTargetWriter : public MCELFObjectTargetWriter {
public:
  LC32ELFObjectTargetWriter(uint8_t OSABI);
  unsigned getRelocType(MCContext &Ctx, const MCValue &Target,
                        const MCFixup &Fixup, bool IsPCRel) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32ELFOBJECTTARGETWRITER_H
