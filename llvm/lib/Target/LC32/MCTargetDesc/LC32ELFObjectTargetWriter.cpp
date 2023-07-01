//===-- LC32ELFObjectWriter.h - LC-3.2 ELF Writer -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ELFObjectTargetWriter.h"
#include "LC32FixupKinds.h"
#include "llvm/MC/MCContext.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32ELFObjectTargetWriter"

LC32ELFObjectTargetWriter::LC32ELFObjectTargetWriter(uint8_t OSABI)
    : MCELFObjectTargetWriter(false, OSABI, ELF::EM_LC_3_2, true) {}

unsigned LC32ELFObjectTargetWriter::getRelocType(MCContext &Ctx,
                                                 const MCValue &Target,
                                                 const MCFixup &Fixup,
                                                 bool IsPCRel) const {
  // For each fixup, emit the relocation it corresponds to
  // If there's no corresponding relocation, die
  switch (Fixup.getKind()) {
  case FK_NONE:
    return ELF::R_LC_3_2_NONE;
  case FK_Data_4:
    return ELF::R_LC_3_2_32;
  case TFK_PCOffset9BR:
    Ctx.reportError(Fixup.getLoc(), "PCOffset9BR has no relocation");
    return ELF::R_LC_3_2_NONE;
  default:
    llvm_unreachable("No relocation for fixup");
  }
}
