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
#include "llvm/Support/Format.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
#define DEBUG_TYPE "LC32Disassembler"

// So we don't have to keep typing the long name
typedef MCDisassembler::DecodeStatus DecodeStatus;

LC32Disassembler::LC32Disassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
    : MCDisassembler(STI, Ctx) {}

uint64_t LC32Disassembler::suggestBytesToSkip(ArrayRef<uint8_t> Bytes,
                                              uint64_t Address) const {
  // Instructions are two byte aligned
  return 2;
}

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

static DecodeStatus DecodeAmount3(MCInst &MI, uint64_t Imm, uint64_t Address,
                                  const MCDisassembler *Decoder) {
  MI.addOperand(MCOperand::createImm(Imm + 1));
  return DecodeStatus::Success;
}

template <unsigned N, unsigned S>
static DecodeStatus DecodePCOffset(MCInst &MI, uint64_t Imm, uint64_t Address,
                                   const MCDisassembler *Decoder) {
  // Check if this is an address we know
  bool sym_worked =
      Decoder->tryAddingSymbolicOperand(MI, Address + (Imm << S) + 2, Address,
                                        MI.getOpcode() == LC32::BR, 0, 2, 2);
  // Otherwise, create an immediate
  if (!sym_worked)
    MI.addOperand(MCOperand::createImm(SignExtend64<N + S>((Imm << S)) + 2));
  // Done
  return DecodeStatus::Success;
}

// Functions to annotate pseudo instructions in the comment stream. These
// functions return `true` if they match. If they don't match, further pseudo
// instructions are tried.
// See: LC32MCCodeEmitter::encodeInstruction
namespace {

bool annotateP_LOADCONSTH(ArrayRef<uint8_t> Bytes, raw_ostream &CStream) {

  // Fetch the halfwords from the instruction
  // We know that its in bounds since the caller checks that
  const uint16_t lea_raw = (Bytes[1] << 8) + Bytes[0];
  const uint16_t ldh_raw = (Bytes[3] << 8) + Bytes[2];
  const uint16_t br_raw = (Bytes[5] << 8) + Bytes[4];

  // Mask out the registers from the raw data
  const uint16_t lea_masked = lea_raw & 0xf1ff;
  const uint16_t ldh_masked = ldh_raw & 0xf03f;
  const uint16_t br_masked = br_raw;

  // Pull out the registers from the data
  const unsigned lea_reg_dr = (lea_raw & 0x0e00) >> 9;
  const unsigned ldh_reg_dr = (ldh_raw & 0x0e00) >> 9;
  const unsigned ldh_reg_baser = (ldh_raw & 0x01c0) >> 6;

  // Assert all the masked values are what we expect, and that all the registers
  // are equal to each other
  if (lea_masked != 0xe004 || ldh_masked != 0x6000 || br_masked != 0x0e01 ||
      lea_reg_dr != ldh_reg_dr || ldh_reg_dr != ldh_reg_baser)
    return false;

  // Get the data being loaded
  const int16_t data = (Bytes[7] << 8) + Bytes[6];
  // Insert the comment and return
  CStream << "PSEUDO.LOADCONSTH R" << lea_reg_dr << ", #" << data;
  return true;
}

bool annotateP_LOADCONSTW(ArrayRef<uint8_t> Bytes, raw_ostream &CStream) {

  // Fetch the halfwords from the instruction
  // We know that its in bounds since the caller checks that
  const uint16_t lea_raw = (Bytes[1] << 8) + Bytes[0];
  const uint16_t ldw_raw = (Bytes[3] << 8) + Bytes[2];
  const uint16_t br_raw = (Bytes[5] << 8) + Bytes[4];
  const uint16_t trap_enc = (Bytes[7] << 8) + Bytes[6];

  // Mask out the registers from the raw data
  const uint16_t lea_masked = lea_raw & 0xf1ff;
  const uint16_t ldw_masked = ldw_raw & 0xf03f;
  const uint16_t br_masked = br_raw;
  const uint16_t trap_masked = trap_enc;

  // Pull out the registers from the data
  const unsigned lea_reg_dr = (lea_raw & 0x0e00) >> 9;
  const unsigned ldw_reg_dr = (ldw_raw & 0x0e00) >> 9;
  const unsigned ldw_reg_baser = (ldw_raw & 0x01c0) >> 6;

  // Assert all the masked values are what we expect, and that all the registers
  // are equal to each other
  if (lea_masked != 0xe006 || ldw_masked != 0xa000 || br_masked != 0x0e03 ||
      trap_masked != 0xf0ff || lea_reg_dr != ldw_reg_dr ||
      ldw_reg_dr != ldw_reg_baser)
    return false;

  // Get the data being loaded
  const int32_t data =
      (Bytes[11] << 24) + (Bytes[10] << 16) + (Bytes[9] << 8) + Bytes[8];
  // Insert the comment and return
  CStream << "PSEUDO.LOADCONSTW R" << lea_reg_dr << ", #" << data;
  return true;
}

bool annotateP_FARJSR(ArrayRef<uint8_t> Bytes, raw_ostream &CStream) {

  // Fetch the halfwords from the instruction
  // We know that its in bounds since the caller checks that
  const uint16_t lea_enc = (Bytes[1] << 8) + Bytes[0];
  const uint16_t ldw_enc = (Bytes[3] << 8) + Bytes[2];
  const uint16_t jsrr_enc = (Bytes[5] << 8) + Bytes[4];
  const uint16_t br_enc = (Bytes[7] << 8) + Bytes[6];

  // Assert all the values are what we expect
  if (lea_enc != 0xe606 || ldw_enc != 0xa6c0 || jsrr_enc != 0x40c0 ||
      br_enc != 0x0e02)
    return false;

  // Get the call target
  const uint32_t target =
      (Bytes[11] << 24) + (Bytes[10] << 16) + (Bytes[9] << 8) + Bytes[8];
  // Insert the comment and return
  CStream << "PSEUDO.FARJSR x" << format_hex_no_prefix(target, 8);
  return true;
}

} // namespace

// Provides: decodeInstruction
// Requires: DecodeGPRRegisterClass
// Requires: DecodeShiftedSignedImm
// Requires: DecodePCOffset
#include "LC32GenDisassemblerTables.inc"

DecodeStatus LC32Disassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                              ArrayRef<uint8_t> Bytes,
                                              uint64_t Address,
                                              raw_ostream &CStream) const {
  // Size is an out parameter
  Size = 0;

  // Smallest instruction is two bytes
  // Can't do less than that
  if (Bytes.size() < 2)
    return DecodeStatus::Fail;

  // Add comments for pseudo instructions
  // Do it at most one time
  // See: LC32MCCodeEmitter::encodeInstruction
  {
    bool keep_trying = true;
    // Process long instructions first
    if (keep_trying && Bytes.size() >= 12)
      keep_trying = !annotateP_LOADCONSTW(Bytes, CStream);
    if (keep_trying && Bytes.size() >= 12)
      keep_trying = !annotateP_FARJSR(Bytes, CStream);
    // Then process short instructions
    if (keep_trying && Bytes.size() >= 8)
      keep_trying = !annotateP_LOADCONSTH(Bytes, CStream);
  }

  // Normal instruction
  // Read the next two bytes and decode with TableGen
  Size = 2;
  uint16_t instr = support::endian::read16le(Bytes.data());
  return decodeInstruction(DecoderTable16, MI, instr, Address, this, this->STI);
}
