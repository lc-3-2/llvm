//===-- LC32DisassemblerImpl.cpp - Disassembler Implementation ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32Disassembler.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/MC/MCDecoderOps.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/Endian.h"
#include "llvm/Support/MathExtras.h"
using namespace llvm;
using namespace llvm::LC32;
#define DEBUG_TYPE "LC32Disassembler"

// So we don't have to keep typing the long name
typedef MCDisassembler::DecodeStatus DecodeStatus;

LC32Disassembler::LC32Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
    : MCDisassembler(STI, Ctx) {}

static DecodeStatus DecodeGPRRegisterClass(MCInst &MI, uint64_t RegNo,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  assert(RegNo <= 7 && "Bad register number");
  // TableGen allocates its own numbers to registers, so we need to translate
  // using a lookup table.
  static const unsigned lut[] = {LC32::R0, LC32::R1, LC32::R2, LC32::AT,
                                 LC32::GP, LC32::FP, LC32::SP, LC32::LR};
  MI.addOperand(MCOperand::createReg(lut[RegNo]));
  return DecodeStatus::Success;
}

template <unsigned N, unsigned S>
static DecodeStatus DecodeShiftedSignedImm(MCInst &MI, uint64_t Imm,
                                           uint64_t Address,
                                           const MCDisassembler *Decoder) {
  MI.addOperand(MCOperand::createImm(SignExtend64<N + S>(Imm << S)));
  return DecodeStatus::Success;
}

// Provides: decodeInstruction
// Requires: DecodeGPRRegisterClass
// Requires: DecodeShiftedSignedImm
#include "LC32GenDisassemblerTables.inc"

DecodeStatus LC32Disassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  // Smallest instruction is two bytes
  // Can't do less than that
  if (Bytes.size() < 2)
    return DecodeStatus::Fail;

  // Normal instruction
  // Read the next two bytes and decode with TableGen
  uint16_t instr = support::endian::read16le(Bytes.data());
  return decodeInstruction(DecoderTable16, MI, instr, Address, this, this->STI);
}
