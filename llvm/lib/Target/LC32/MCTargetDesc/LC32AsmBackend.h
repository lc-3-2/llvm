//===-- LC32AsmBackend.cpp - LC-3.2 Assembler Backend ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class deals with converting assembly files to object files. It deals
// with alignment and relaxation mostly. A lot of the fixup code was taken from
// the MSP430 backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMBACKEND_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMBACKEND_H

#include "llvm/MC/MCAsmBackend.h"

namespace llvm {
class MCContext;

class LC32AsmBackend : public MCAsmBackend {
public:
  LC32AsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI);

  // Creates the LC32ELFObjectTargetWriter
  std::unique_ptr<MCObjectTargetWriter>
  createObjectTargetWriter() const override;

  unsigned getMinimumNopSize() const override;
  bool writeNopData(raw_ostream &OS, uint64_t Count,
                    const MCSubtargetInfo *STI) const override;

  // This function emits alignment at the start of each instruction
  void emitInstructionBegin(MCObjectStreamer &OS, const MCInst &Inst,
                            const MCSubtargetInfo &STI) override;

  // Fixup Interface
  unsigned getNumFixupKinds() const override;
  const MCFixupKindInfo &getFixupKindInfo(MCFixupKind Kind) const override;
  void applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                  const MCValue &Target, MutableArrayRef<char> Data,
                  uint64_t Value, bool IsResolved,
                  const MCSubtargetInfo *STI) const override;

  // Relaxation Interface
  bool fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                            const MCRelaxableFragment *DF,
                            const MCAsmLayout &Layout) const override;
  void relaxInstruction(MCInst &Inst,
                        const MCSubtargetInfo &STI) const override;

private:
  // For constructing LC32ELFObjectTargetWriter
  uint8_t OSABI;

  /** \brief Helper function for computing the encoding of fixup values in
   * instructions
   *
   * See the MSP430 backend for where this was inspired from.
   *
   * \param [in] Fixup The `MCFixup` to apply
   * \param [in] Value The value to apply with the fixup
   * \param [in] Ctx Assembler context for errors
   * \return The encoded value
   */
  uint64_t adjustFixupValue(MCFixupKind Kind, uint64_t Value,
                            MCContext &Ctx) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCASMBACKEND_H
