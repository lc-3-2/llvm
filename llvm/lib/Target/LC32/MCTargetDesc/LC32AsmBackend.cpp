//===-- LC32AsmBackend.cpp - LC-3.2 Assembler Backend ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32AsmBackend.h"
#include "LC32ELFObjectTargetWriter.h"
#include "LC32FixupKinds.h"
#include "LC32MCTargetDesc.h"
#include "llvm/MC/MCAssembler.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCFixupKindInfo.h"
#include "llvm/MC/MCObjectStreamer.h"
#include "llvm/Support/Alignment.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmBackend"

LC32AsmBackend::LC32AsmBackend(const MCSubtargetInfo &STI, uint8_t OSABI)
    : MCAsmBackend(support::little), OSABI(OSABI) {}

std::unique_ptr<MCObjectTargetWriter>
LC32AsmBackend::createObjectTargetWriter() const {
  return std::make_unique<LC32ELFObjectTargetWriter>(this->OSABI);
}

unsigned LC32AsmBackend::getMinimumNopSize() const {
  // NOPs are always multiples of two bytes
  return 2;
}
bool LC32AsmBackend::writeNopData(raw_ostream &OS, uint64_t Count,
                                  const MCSubtargetInfo *STI) const {
  // NOPs are always multiples of two bytes
  // Fail if the Count isn't
  if (Count % 2 != 0)
    return false;
  // Otherwise, NOPs are zeros
  OS.write_zeros(Count);
  return true;
}

void LC32AsmBackend::emitInstructionBegin(MCObjectStreamer &OS,
                                          const MCInst &Inst,
                                          const MCSubtargetInfo &STI) {
  // Long pseudo instructions are aligned to four bytes
  // Non-pseudo instructions are aligned to two bytes
  // Emit zeros until then
  if (Inst.getOpcode() == LC32::P_LOADCONSTW)
    OS.emitValueToAlignment(Align(4));
  else
    OS.emitValueToAlignment(Align(2));
}

unsigned LC32AsmBackend::getNumFixupKinds() const {
  return NumTargetFixupKinds;
}

const MCFixupKindInfo &
LC32AsmBackend::getFixupKindInfo(MCFixupKind Kind) const {

  // Table for all the fixup information
  // Needed since we have to return a reference
  // Must match LC32FixupKinds.h
  const static MCFixupKindInfo Infos[] = {
      {"TFK_PCOffset9", 0, 9, MCFixupKindInfo::FKF_IsPCRel},
      {"TFK_PCOffset11", 0, 11, MCFixupKindInfo::FKF_IsPCRel},
  };
  static_assert(std::size(Infos) == NumTargetFixupKinds,
                "Not all fixup kinds added to Infos array");

  // If it's an LLVM-defined fixup, handle it
  if (Kind < FirstTargetFixupKind)
    return MCAsmBackend::getFixupKindInfo(Kind);

  // Otherwise, return our information
  assert(Kind - FirstTargetFixupKind < NumTargetFixupKinds &&
         "Unknown target kind");
  return Infos[Kind - FirstTargetFixupKind];
}

void LC32AsmBackend::applyFixup(const MCAssembler &Asm, const MCFixup &Fixup,
                                const MCValue &Target,
                                MutableArrayRef<char> Data, uint64_t Value,
                                bool IsResolved,
                                const MCSubtargetInfo *STI) const {
  // Compute the actual fixup value
  // If it doesn't change the encoding, don't do anything
  Value = this->adjustFixupValue(Fixup, Value, Asm.getContext());

  // Get the information for the fixup
  // How many bits and where to start
  MCFixupKindInfo info = this->getFixupKindInfo(Fixup.getKind());

  // Figure out what bytes are affected
  size_t byte_offset = Fixup.getOffset();
  size_t bit_offset = info.TargetOffset;
  size_t num_bytes = (info.TargetSize + bit_offset + 7) / 8;
  assert(byte_offset + num_bytes <= Data.size() && "Not enough Data for fixup");

  // Do the change
  Value <<= bit_offset;
  for (size_t i = 0; i < num_bytes; i++) {
    Data[byte_offset + i] |= (Value >> (i << 3)) & 0xff;
  }
}
uint64_t LC32AsmBackend::adjustFixupValue(const MCFixup &Fixup, uint64_t Value,
                                          MCContext &Ctx) const {
  // Remember, this code has to work for LLVM-defined fixups too
  // Thus, have a default case where we just return Value
  unsigned Kind = Fixup.getKind();
  switch (Kind) {

  case TFK_PCOffset9:
  case TFK_PCOffset11: {
    // Check that the preconditions are met
    if ((Value & 0x1) != 0)
      Ctx.reportError(Fixup.getLoc(), "PCOffset not halfword aligned");
    if (Kind == TFK_PCOffset9 && !isShiftedInt<9, 1>(Value - 2))
      Ctx.reportError(Fixup.getLoc(), "PCOffset9 out of range");
    if (Kind == TFK_PCOffset11 && !isShiftedInt<11, 1>(Value - 2))
      Ctx.reportError(Fixup.getLoc(), "PCOffset11 out of range");
    // Compute the offset and return
    int64_t Offset = Value;
    Offset -= 2;
    Offset >>= 1;
    if (Kind == TFK_PCOffset9)
      Offset &= 0x1ff;
    if (Kind == TFK_PCOffset11)
      Offset &= 0x7ff;
    return Offset;
  }

  default:
    return Value;
  }
}

bool LC32AsmBackend::mayNeedRelaxation(const MCInst &Inst,
                                       const MCSubtargetInfo &STI) const {
  switch (Inst.getOpcode()) {
  case LC32::LEA:
  case LC32::JSR:
    return true;
  default:
    return false;
  }
}

bool LC32AsmBackend::fixupNeedsRelaxation(const MCFixup &Fixup, uint64_t Value,
                                          const MCRelaxableFragment *DF,
                                          const MCAsmLayout &Layout) const {
  unsigned Kind = Fixup.getKind();
  switch (Kind) {
  case TFK_PCOffset9:
    return !isShiftedInt<9, 1>(Value - 2);
  case TFK_PCOffset11:
    return !isShiftedInt<11, 1>(Value - 2);
  default:
    return false;
  }
}

void LC32AsmBackend::relaxInstruction(MCInst &Inst,
                                      const MCSubtargetInfo &STI) const {

  switch (Inst.getOpcode()) {
  case LC32::LEA:
    // LEA -> PSEUDO.LOADCONSTW
    Inst.setOpcode(LC32::P_LOADCONSTW);
    return;
  default:
    llvm_unreachable("Bad instruction to relax");
  }
}
