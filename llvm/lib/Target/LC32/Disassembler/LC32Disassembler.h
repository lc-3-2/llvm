//===-- LC32Disassembler.cpp - Disassembler for LC-3.2 --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_DISASSEMBLER_LC32DISASSEMBLER_H
#define LLVM_LIB_TARGET_LC32_DISASSEMBLER_LC32DISASSEMBLER_H

#include "llvm/MC/MCDisassembler/MCDisassembler.h"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32Disassembler : public MCDisassembler {
public:
  LC32Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx);

  // Decode a single instruction from bytes
  // This function calls into TableGen, but that assumes the size is known. This
  // instruction determines the size before calling.
  DecodeStatus getInstruction(MCInst &MI, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &CStream) const override;
};

} // namespace llvm::lc32

#endif // LLVM_LIB_TARGET_LC32_DISASSEMBLER_LC32DISASSEMBLER_H
