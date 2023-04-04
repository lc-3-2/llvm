//===-- LC32ELFObjectWriter.h - LC-3.2 ELF Writer -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ELFObjectTargetWriter.h"
#include "LC32FixupKinds.h"
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
  switch (Fixup.getTargetKind()) {
    case FIXUP_32:
      assert(!IsPCRel && "Bad fixup kind for PC-relative");
      return ELF::R_LC_3_2_32;
  }
  // If there's no corresponding relocation, die
  llvm_unreachable("No relocation for fixup");
}

std::unique_ptr<MCObjectTargetWriter> createLC32ELFObjectWriter(uint8_t OSABI) {
  return std::make_unique<LC32ELFObjectTargetWriter>(OSABI);
}
