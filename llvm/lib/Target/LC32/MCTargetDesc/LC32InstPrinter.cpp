//= LC32InstPrinter.cpp - Print LC-3.2 MCInst in Assembly Syntax ---*- C++ -*-//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstPrinter.h"
#include "LC32MCTargetDesc.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/MathExtras.h"
#include <cassert>
using namespace llvm;
#define DEBUG_TYPE "LC32MCInstPrinter"

// Provides: printInstruction
// Provides: printAliasInstr
// Provides: getRegisterName
// Requires: printOperand
// Requires: printShiftedSignedImmOperand
// Requires: printPCOffset
#define PRINT_ALIAS_INSTR
#include "LC32GenAsmWriter.inc"

LC32InstPrinter::LC32InstPrinter(const MCAsmInfo &MAI, const MCInstrInfo &MII,
                                 const MCRegisterInfo &MRI)
    : MCInstPrinter(MAI, MII, MRI) {}

void LC32InstPrinter::printInst(const MCInst *MI, uint64_t Address,
                                StringRef Annot, const MCSubtargetInfo &STI,
                                raw_ostream &OS) {
  // Print annotation if needed
  this->printAnnotation(OS, Annot);

  // Special handling for NOP
  if (MI->getOpcode() == LC32::BR && MI->getOperand(0).getImm() == 0b000) {
    OS << "NOP";
    return;
  }

  // If no special handling, call TableGen
  // Try aliases first
  {
    bool alias_worked = this->printAliasInstr(MI, Address, OS);
    if (!alias_worked)
      this->printInstruction(MI, Address, OS);
  }
}

void LC32InstPrinter::printOperand(const MCInst *MI, unsigned OpNo,
                                   raw_ostream &O, const char *Modifier) {
  // Get the operand
  const MCOperand &op = MI->getOperand(OpNo);

  // Print registers
  // Call TableGen to decode the number to a name
  if (op.isReg()) {
    O << getRegisterName(op.getReg());
    return;
  }

  // Print immediates in decimal
  // Prefix them with a #
  if (op.isImm()) {
    O << '#' << op.getImm();
    return;
  }

  llvm_unreachable("Unknown operand type");
}

template <unsigned N, unsigned S>
void LC32InstPrinter::printShiftedSignedImmOperand(const MCInst *MI,
                                                   unsigned OpNo,
                                                   raw_ostream &O,
                                                   const char *Modifier) {
  // Get the operand
  const MCOperand &op = MI->getOperand(OpNo);
  // Check operand has the right form
  // Get around commas breaking assert
  {
    assert(op.isImm() && "Not an immediate");
    bool is_correct = isShiftedInt<N, S>(op.getImm());
    assert(is_correct && "Bad value for immediate");
  }
  // Handle immediate shifting
  // Prefix them with a #
  O << '#' << (op.getImm() >> S);
}

template <unsigned N>
void LC32InstPrinter::printPCOffset(const MCInst *MI, unsigned OpNo,
                                    raw_ostream &O, const char *Modifier) {
  // Get the operand
  const MCOperand &op = MI->getOperand(OpNo);

  // Immediate case
  if (op.isImm()) {
    // Correct for PC
    int64_t offset_imm = op.getImm() - 2;
    // Check operand has right form
    // Get around commas breaking assert
    {
      bool is_correct = isShiftedInt<N, 1>(offset_imm);
      assert(is_correct && "Bad value for immediate");
    }
    // Print
    O << '#' << (offset_imm >> 1);
    return;
  }

  // Expression case
  if (op.isExpr()) {
    op.getExpr()->print(O, &this->MAI);
    return;
  }

  llvm_unreachable("Bad operand type for PC offset");
}

void LC32InstPrinter::printNZP(const MCInst *MI, unsigned OpNo, raw_ostream &O,
                               const char *Modifier) {
  // Get the operand
  const MCOperand &op = MI->getOperand(OpNo);
  // Check operand has right form
  assert(op.isImm() && "NZP operands must be immediates");
  // Print
  switch (op.getImm()) {
  case 0b100:
    O << "n";
    return;
  case 0b010:
    O << "z";
    return;
  case 0b001:
    O << "p";
    return;
  case 0b110:
    O << "nz";
    return;
  case 0b101:
    O << "np";
    return;
  case 0b011:
    O << "zp";
    return;
  case 0b111:
    O << "nzp";
    return;
  default:
    llvm_unreachable("NZP operand out of range for printing");
  }
}
