//===-- LC32MCCodeEmitter.h - Convert LC-3.2 MCInst to machine code -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module writes MCInsts to output in machine code format. The main
// interface is encodeInstruction, and the main TableGen function is
// getBinaryCodeForInstr.
//
// See: LC32GenMCCodeEmitter.inc
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCCODEEMITTER_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCCODEEMITTER_H

#include "llvm/MC/MCCodeEmitter.h"
#include <cstdint>

namespace llvm {
class MCContext;
class MCInstrInfo;
class MCOperand;

class LC32MCCodeEmitter : public MCCodeEmitter {
public:
  LC32MCCodeEmitter(MCContext &Ctx, MCInstrInfo const &MCII);
  void encodeInstruction(const MCInst &Inst, raw_ostream &OS,
                         SmallVectorImpl<MCFixup> &Fixups,
                         const MCSubtargetInfo &STI) const override;

private:
  // TableGen
  // Called by encodeInstruction
  uint64_t getBinaryCodeForInstr(const MCInst &MI,
                                 SmallVectorImpl<MCFixup> &Fixups,
                                 const MCSubtargetInfo &STI) const;

  // Context for errors and register encoding
  MCContext &Ctx;

  // Operand encoding instructions required by TableGen
  uint64_t getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                             SmallVectorImpl<MCFixup> &Fixups,
                             const MCSubtargetInfo &STI) const;
  template <unsigned N, unsigned S>
  uint64_t getShiftedSignedImmOpValue(const MCInst &MI, unsigned OpNo,
                                      SmallVectorImpl<MCFixup> &Fixups,
                                      const MCSubtargetInfo &STI) const;
  template <unsigned N, unsigned S>
  uint64_t getPCOffsetValue(const MCInst &MI, unsigned OpNo,
                            SmallVectorImpl<MCFixup> &Fixups,
                            const MCSubtargetInfo &STI) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCCODEEMITTER_H
