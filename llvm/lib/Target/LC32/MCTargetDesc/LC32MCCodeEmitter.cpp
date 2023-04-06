//===-- LC32MCCodeEmitter.cpp - Convert LC-3.2 MCInst to machine code -----===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32MCCodeEmitter.h"
#include "LC32MCTargetDesc.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/Support/EndianStream.h"
using namespace llvm;
#define DEBUG_TYPE "LC32MCCodeEmitter"

// Provides: getBinaryCodeForInstr
// Requires: getMachineOpValue
// Requires: getShiftedSignedImmOpValue
#include "LC32GenMCCodeEmitter.inc"

LC32MCCodeEmitter::LC32MCCodeEmitter(MCContext &Ctx, MCInstrInfo const &MCII)
    : Ctx(Ctx) {}

void LC32MCCodeEmitter::encodeInstruction(const MCInst &Inst, raw_ostream &OS,
                                          SmallVectorImpl<MCFixup> &Fixups,
                                          const MCSubtargetInfo &STI) const {
  // Non-pseudo instructions are just written as-is
  // Remember to convert to uint16_t so only two bytes are written
  uint64_t enc = this->getBinaryCodeForInstr(Inst, Fixups, STI);
  assert(enc < 0x10000 && "Instructions must be 16-bit");
  support::endian::write(OS, static_cast<uint16_t>(enc),
                         support::endianness::little);
}

uint64_t
LC32MCCodeEmitter::getMachineOpValue(const MCInst &MI, const MCOperand &MO,
                                     SmallVectorImpl<MCFixup> &Fixups,
                                     const MCSubtargetInfo &STI) const {
  // Handle registers
  if (MO.isReg())
    return this->Ctx.getRegisterInfo()->getEncodingValue(MO.getReg());

  // Encode immediates as-is
  if (MO.isImm())
    return MO.getImm();

  llvm_unreachable("Unknown operand type");
}

template <unsigned N, unsigned S>
uint64_t LC32MCCodeEmitter::getShiftedSignedImmOpValue(
    const MCInst &MI, unsigned OpNo, SmallVectorImpl<MCFixup> &Fixups,
    const MCSubtargetInfo &STI) const {
  // Get the operand
  const MCOperand &op = MI.getOperand(OpNo);
  // Check operand has the right form
  // Get around commas breaking assert
  {
    assert(op.isImm() && "Not an immediate");
    bool is_correct = isShiftedInt<N, S>(op.getImm());
    assert(is_correct && "Bad value for immediate");
  }
  // Return
  return op.getImm() >> S;
}
