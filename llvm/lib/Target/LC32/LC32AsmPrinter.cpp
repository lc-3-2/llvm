//===-- LC32AsmPrinter.cpp - LC-3.2 LLVM Assembly Writer ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file handles converting MachineInstrs to MCInsts. In the process, it
// gets rid of all the different types of addresses and replaces them with
// symbols. This way, they can be fed into the MC layer.
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/TargetSubtargetInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/MC/MCContext.h"
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

  // Utility methods
  MCOperand lowerSymbolOperand(MachineOperand MO, MCSymbol *Sym);
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
  if (MI->getOpcode() == LC32::C_BR_CMP_ZERO) {
    MCInst temp_add;
    MCInst temp_br;
    MCOperand nzp;
    MCOperand sr;
    MCOperand target;
    this->lowerOperand(MI->getOperand(0), nzp);
    this->lowerOperand(MI->getOperand(1), sr);
    this->lowerOperand(MI->getOperand(2), target);
    temp_add.setOpcode(LC32::ADDi);
    temp_add.addOperand(sr);
    temp_add.addOperand(sr);
    temp_add.addOperand(MCOperand::createImm(0));
    temp_br.setOpcode(LC32::BR);
    temp_br.addOperand(nzp);
    temp_br.addOperand(target);
    this->EmitToStreamer(*this->OutStreamer, temp_add);
    this->EmitToStreamer(*this->OutStreamer, temp_br);
    return;
  }

  // Lower as-is
  // Usually, this would be placed in a separate file, but we don't really need
  // that
  MCInst temp_instr;
  temp_instr.setOpcode(MI->getOpcode());
  for (const auto &mo : MI->operands()) {
    // Iterate over all the operands and lower them
    // Only add the operand if it was set
    MCOperand temp_mcop;
    this->lowerOperand(mo, temp_mcop);
    if (temp_mcop.isValid())
      temp_instr.addOperand(temp_mcop);
  }
  this->EmitToStreamer(*this->OutStreamer, temp_instr);
}

void LC32AsmPrinter::lowerOperand(MachineOperand MO, MCOperand &MCOp) {
  // Lower a single operand
  // Usually, this would be placed in a separate file, but we don't really need
  // that.
  SmallString<256> symbol_name;
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

  // Lower branch targets
  case MachineOperand::MO_MachineBasicBlock:
    MCOp = this->lowerSymbolOperand(MO, MO.getMBB()->getSymbol());
    return;

  // Lower global addresses
  case MachineOperand::MO_GlobalAddress:
    MCOp = this->lowerSymbolOperand(MO, this->getSymbol(MO.getGlobal()));
    return;

  // Lower external addresses
  case MachineOperand::MO_ExternalSymbol:
    MCOp = this->lowerSymbolOperand(
        MO, this->GetExternalSymbolSymbol(MO.getSymbolName()));
    return;

  // Lower constant pool references
  case MachineOperand::MO_ConstantPoolIndex:
    // Compute the name
    raw_svector_ostream(symbol_name)
        << this->getDataLayout().getPrivateGlobalPrefix()
        << "ConstantPoolIndex_" << this->getFunctionNumber() << '_'
        << MO.getIndex();
    // Return
    MCOp = this->lowerSymbolOperand(
        MO, this->OutContext.getOrCreateSymbol(symbol_name));
    return;

  // Lower jump table references
  case MachineOperand::MO_JumpTableIndex:
    // Compute the name
    raw_svector_ostream(symbol_name)
        << this->getDataLayout().getPrivateGlobalPrefix() << "JumpTableIndex_"
        << this->getFunctionNumber() << '_' << MO.getIndex();
    // Return
    MCOp = this->lowerSymbolOperand(
        MO, this->OutContext.getOrCreateSymbol(symbol_name));
    return;

  // Lower block addresses
  case MachineOperand::MO_BlockAddress:
    MCOp = this->lowerSymbolOperand(
        MO, this->GetBlockAddressSymbol(MO.getBlockAddress()));
    return;
  }
}

MCOperand LC32AsmPrinter::lowerSymbolOperand(MachineOperand MO, MCSymbol *Sym) {
  // Check the flags
  assert(MO.getTargetFlags() == 0 && "Operand target flags should be zero");
  // Create the base symbol
  const MCExpr *expr = MCSymbolRefExpr::create(Sym, this->OutContext);
  // Add any offset if present
  // Some operand classes don't have this method, so check for that
  if (!MO.isMBB() && !MO.isJTI() && MO.getOffset() != 0)
    expr = MCBinaryExpr::createAdd(
        expr, MCConstantExpr::create(MO.getOffset(), this->OutContext),
        this->OutContext);
  // Done
  return MCOperand::createExpr(expr);
}
