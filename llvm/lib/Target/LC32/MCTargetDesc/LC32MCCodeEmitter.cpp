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
  // Handle pseudo instructions
  if (Inst.getOpcode() == LC32::P_LOADCONSTH) {
    // Get the operands
    uint64_t dr =
        this->getMachineOpValue(Inst, Inst.getOperand(0), Fixups, STI);
    uint64_t imm16 =
        this->getMachineOpValue(Inst, Inst.getOperand(1), Fixups, STI);
    // Construct encodings
    uint16_t lea_enc = 0xe002 | (dr << 9);
    uint16_t ldh_enc = 0x6000 | (dr << 9) | (dr << 6);
    uint16_t br_enc = 0x0e01;
    // Write
    support::endian::write(OS, lea_enc, support::endianness::little);
    support::endian::write(OS, ldh_enc, support::endianness::little);
    support::endian::write(OS, br_enc, support::endianness::little);
    support::endian::write(OS, static_cast<uint16_t>(imm16),
                           support::endianness::little);
    // Done
    return;
  }
  if (Inst.getOpcode() == LC32::P_LOADCONSTW) {
    // Get the operands
    uint64_t dr =
        this->getMachineOpValue(Inst, Inst.getOperand(0), Fixups, STI);
    uint64_t imm32 =
        this->getMachineOpValue(Inst, Inst.getOperand(1), Fixups, STI);
    // Construct encodings
    uint16_t lea_enc = 0xe002 | (dr << 9);
    uint16_t ldw_enc = 0xa000 | (dr << 9) | (dr << 6);
    uint16_t br_enc = 0x0e01;
    uint16_t trap_enc = 0xf0ff;
    // Write
    support::endian::write(OS, lea_enc, support::endianness::little);
    support::endian::write(OS, ldw_enc, support::endianness::little);
    support::endian::write(OS, br_enc, support::endianness::little);
    support::endian::write(OS, trap_enc, support::endianness::little);
    support::endian::write(OS, static_cast<uint32_t>(imm32),
                           support::endianness::little);
    // Done
    return;
  }

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

template <unsigned N>
uint64_t LC32MCCodeEmitter::getPCOffsetValue(const MCInst &MI, unsigned OpNo,
                                             SmallVectorImpl<MCFixup> &Fixups,
                                             const MCSubtargetInfo &STI) const {
  llvm_unreachable("TODO");
}
