//===- LC32AsmParserImpl.cpp - Parser Implementation ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32AsmParser.h"
#include "LC32Operand.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace llvm::LC32;
#define DEBUG_TYPE "LC32AsmParser"

// Provides: ComputeAvailableFeatures
#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_CHECKER
#define GET_MNEMONIC_SPELL_CHECKER
#include "LC32GenAsmMatcher.inc"

void LC32Operand::anchor() {}

MCAsmParser &LC32AsmParser::getParser() const { return this->Parser; }
MCAsmLexer &LC32AsmParser::getLexer() const { return this->Parser.getLexer(); }

LC32AsmParser::LC32AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII,
                             const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII), Parser(Parser), STI(STI) {
  // Set the available features
  // We don't have any, but TableGen (and other modules) still query it
  this->setAvailableFeatures(
      this->ComputeAvailableFeatures(STI.getFeatureBits()));
}

OperandMatchResultTy LC32AsmParser::tryParseRegister(MCRegister &Reg,
                                                     SMLoc &StartLoc,
                                                     SMLoc &EndLoc) {
  // The next token must be an unquoted identifier
  if (!this->getLexer().is(AsmToken::Identifier))
    return MatchOperand_NoMatch;

  // Get the string
  AsmToken name_tok = this->getLexer().getTok();
  StringRef name = name_tok.getIdentifier().upper();
  // Get the register number
  // Note that this is the internal register number
  unsigned regno = LC32::NoRegister;
  if (regno == LC32::NoRegister)
    regno = MatchRegisterName(name);
  if (regno == LC32::NoRegister)
    regno = MatchRegisterAltName(name);

  // If we still don't have a register, die
  if (regno == LC32::NoRegister)
    return MatchOperand_ParseFail;
  // Otherwise, set the location and succeed
  Reg = regno;
  StartLoc = name_tok.getLoc();
  EndLoc = name_tok.getEndLoc();
  return MatchOperand_Success;
}

bool LC32AsmParser::parseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                  SMLoc &EndLoc) {
  // Run tryParseRegister and check the result
  OperandMatchResultTy res = this->tryParseRegister(Reg, StartLoc, EndLoc);
  switch (res) {
  case MatchOperand_Success:
    return false;
  case MatchOperand_ParseFail:
    return this->Error(this->getLexer().getLoc(), "invalid register name");
  case MatchOperand_NoMatch:
    return this->Error(this->getLexer().getLoc(), "expected a name");
  }
  llvm_unreachable("Invalid OperandMatchResultTy");
}

bool LC32AsmParser::ParseDirective(AsmToken DirectiveID) {
  return true;
}

bool LC32AsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  // TODO: Implement
  return this->Error(NameLoc, "Unimplemented");
}

bool LC32AsmParser::MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                                            OperandVector &Operands,
                                            MCStreamer &Out,
                                            uint64_t &ErrorInfo,
                                            bool MatchingInlineAsm) {
  // Try to match with the given operands
  MCInst instr;
  unsigned res =
      this->MatchInstructionImpl(Operands, instr, ErrorInfo, MatchingInlineAsm);

  switch (res) {

  // By default, emit the instruction
  // Remember to set the location on it
  case Match_Success:
    instr.setLoc(IDLoc);
    Out.emitInstruction(instr, this->STI);
    return false;

  case Match_MnemonicFail:
    return this->Error(IDLoc, "invalid instruction mnemonic");

  // For an invalid operand, ErrorInfo contains the operand index containing the
  // error, or ~0ull if not present.
  case Match_InvalidOperand:
    if (ErrorInfo == 0ull)
      return this->Error(IDLoc, "bad operand value");
    if (ErrorInfo >= Operands.size())
      return this->Error(IDLoc, "too many operands");
    return this->Error(((const LC32Operand &)Operands[ErrorInfo]).getStartLoc(),
                       "bad operand value");

  // If all else fails, return a generic failure
  default:
    return true;
  }
}
