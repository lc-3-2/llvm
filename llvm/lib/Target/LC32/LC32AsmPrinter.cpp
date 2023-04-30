//===-- LC32AsmPrinter.cpp - LC-3.2 LLVM Assembly Writer ------------------===//
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

#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;
#define DEBUG_TYPE "LC32AsmPrinter"

namespace {

class LC32AsmPrinter : public AsmPrinter {
public:
  LC32AsmPrinter(TargetMachine &TM, std::unique_ptr<MCStreamer> Streamer);
  void emitInstruction(const MachineInstr *MI) override;

private:
  // TableGen
  bool emitPseudoExpansionLowering(MCStreamer &OutStreamer,
                                   const MachineInstr *MI);
  void lowerOperand(MachineOperand MO, MCOperand &MCOp);
};

} // namespace

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32AsmPrinter() {
  RegisterAsmPrinter<LC32AsmPrinter> X(getTheLC32Target());
}

LC32AsmPrinter::LC32AsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)) {}

// Provides: emitPseudoExpansionLowering
// Requires: lowerOperand
#include "LC32GenMCPseudoLowering.inc"

void LC32AsmPrinter::emitInstruction(const MachineInstr *MI) {
  // Check predicates
  LC32_MC::verifyInstructionPredicates(
      MI->getOpcode(), this->getSubtargetInfo().getFeatureBits());

  // Try to lower simple pseudo instructions
  // This doesn't handle things that go to two or more instructions. We have to
  // lower them manually.
  if (this->emitPseudoExpansionLowering(*this->OutStreamer, MI))
    return;

  // Lower complex pseudo instructions
  if (MI->getOpcode() == LC32::C_MOVE_IMM5) {
    MCInst temp_and;
    MCInst temp_add;
    MCOperand dr;
    MCOperand imm5;
    this->lowerOperand(MI->getOperand(0), dr);
    this->lowerOperand(MI->getOperand(1), imm5);
    temp_and.setOpcode(LC32::ANDi);
    temp_and.addOperand(dr);
    temp_and.addOperand(dr);
    temp_and.addOperand(MCOperand::createImm(0));
    temp_add.setOpcode(LC32::ADDi);
    temp_add.addOperand(dr);
    temp_add.addOperand(dr);
    temp_add.addOperand(imm5);
    this->EmitToStreamer(*this->OutStreamer, temp_and);
    this->EmitToStreamer(*this->OutStreamer, temp_add);
    return;
  }

  // Lower as-is
  // Usually, this would be placed in a separate file, but we don't really need
  // that
  MCInst temp_instr;
  temp_instr.setOpcode(MI->getOpcode());
  for (const auto &mo : MI->operands()) {
    // Iterate over all the operands and lower them
    MCOperand temp_mcop;
    this->lowerOperand(mo, temp_mcop);
    temp_instr.addOperand(temp_mcop);
  }
  this->EmitToStreamer(*this->OutStreamer, temp_instr);
}

void LC32AsmPrinter::lowerOperand(MachineOperand MO, MCOperand &MCOp) {
  // Lower a single operand
  // Usually, this would be placed in a separate file, but we don't really need
  // that.
  switch (MO.getType()) {
  default:
    llvm_unreachable("Bad operand type");

  // Lower registers
  // Don't explicitly add implicit defs
  case MachineOperand::MO_Register:
    if (MO.isImplicit())
      return;
    MCOp = MCOperand::createReg(MO.getReg());
    return;

  // Lower immediates
  case MachineOperand::MO_Immediate:
    MCOp = MCOperand::createImm(MO.getImm());
    return;

  // Lower global addresses
  case MachineOperand::MO_GlobalAddress:
    // Create the base symbol
    const MCExpr *expr = MCSymbolRefExpr::create(
        this->getSymbol(MO.getGlobal()), this->OutContext);
    // Add any offset if present
    if (MO.getOffset() != 0)
      expr = MCBinaryExpr::createAdd(
          expr, MCConstantExpr::create(MO.getOffset(), this->OutContext),
          this->OutContext);
    // Done
    MCOp = MCOperand::createExpr(expr);
    return;
  }
}
