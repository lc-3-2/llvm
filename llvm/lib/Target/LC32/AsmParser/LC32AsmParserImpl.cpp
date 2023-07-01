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
#include "operand/LC32OperandExpr.h"
#include "operand/LC32OperandImm.h"
#include "operand/LC32OperandNZP.h"
#include "operand/LC32OperandReg.h"
#include "operand/LC32OperandToken.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
#include <optional>
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmParser"

// Provides: ComputeAvailableFeatures
#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#define GET_MNEMONIC_CHECKER
#define GET_MNEMONIC_SPELL_CHECKER
#include "LC32GenAsmMatcher.inc"

const std::vector<std::function<operand_parser_t>> OPERAND_PARSERS = {
    OPERAND_PARSER_REG,
    OPERAND_PARSER_IMM,
    OPERAND_PARSER_EXPR,
};

void LC32Operand::anchor() {}

LC32AsmParser::LC32AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                             const MCInstrInfo &MII,
                             const MCTargetOptions &Options)
    : MCTargetAsmParser(Options, STI, MII), Parser(Parser), STI(STI) {
  // Set the available features
  // We don't have any, but TableGen (and other modules) still query it
  this->setAvailableFeatures(
      this->ComputeAvailableFeatures(STI.getFeatureBits()));

  // Add directive aliases
  // These all get converted to lowercase. Also, we destroy the .string
  // directive, and we don't handle .blkx as an alias.
  // See: MC/MCParser/AsmParser.cpp
  this->getParser().addAliasForDirective(".external", ".extern");
  this->getParser().addAliasForDirective(".fillb", ".byte");
  this->getParser().addAliasForDirective(".fillh", ".short");
  this->getParser().addAliasForDirective(".fillw", ".int");
  this->getParser().addAliasForDirective(".fillq", ".quad");
  this->getParser().addAliasForDirective(".blk", ".fill");
  this->getParser().addAliasForDirective(".string", ".ascii");
  this->getParser().addAliasForDirective(".stringz", ".asciz");
}

MCAsmParser &LC32AsmParser::getParser() const { return this->Parser; }
MCAsmLexer &LC32AsmParser::getLexer() const { return this->Parser.getLexer(); }

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
    return MatchOperand_NoMatch;
  // Otherwise, set the location and succeed
  Reg = regno;
  StartLoc = name_tok.getLoc();
  EndLoc = name_tok.getEndLoc();
  this->getLexer().Lex();
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
    return this->Error(this->getLexer().getLoc(), "expected a register name");
  }
  llvm_unreachable("Invalid OperandMatchResultTy");
}

bool LC32AsmParser::ParseDirective(AsmToken DirectiveID) {
  // Normalize with lowercase
  std::string directive_name = DirectiveID.getIdentifier().lower();

  // Custom handling for .blkx directives
  // Most of the code is taken from the original parser
  // See: MC/MCParser/AsmParser.cpp
  if (directive_name == ".blkb" || directive_name == ".blkh" ||
      directive_name == ".blkw" || directive_name == ".blkq") {

    // Compute the multiplier
    size_t scale = StringSwitch<size_t>(directive_name)
                       .CaseLower(".blkb", 1)
                       .CaseLower(".blkh", 2)
                       .CaseLower(".blkw", 4)
                       .CaseLower(".blkq", 8)
                       .Default(0);
    assert(scale != 0 && "Not all cases handled");

    // Parse the expression
    // Remember the location
    SMLoc count_loc = this->getLexer().getLoc();
    const MCExpr *count;
    if (this->getParser().checkForValidSection() ||
        this->getParser().parseExpression(count)) {
      return true;
    }

    // Write out
    this->getStreamer().emitFill(*count, scale, 0x00, count_loc);
    return false;
  }

  return true;
}

bool LC32AsmParser::ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                                     SMLoc NameLoc, OperandVector &Operands) {
  // Mnemonics ignore case, so convert to lower
  std::string mnemonic = Name.lower();

  // Check for BR instructions
  // Remember that BREAK also starts with BR - ignore that
  if (mnemonic == "nop") {
    Operands.push_back(std::make_unique<LC32OperandToken>("br", NameLoc));
    Operands.push_back(
        std::make_unique<LC32OperandNZP>(0b000, NameLoc, NameLoc));
    Operands.push_back(std::make_unique<LC32OperandImm>(0, NameLoc, NameLoc));

  } else if (mnemonic == "br") {
    Operands.push_back(std::make_unique<LC32OperandToken>(mnemonic, NameLoc));
    Operands.push_back(
        std::make_unique<LC32OperandNZP>(0b111, NameLoc, NameLoc));

  } else if (mnemonic != "break" && mnemonic.substr(0, 2) == "br") {
    // Carve out the part of the mnemonic we need
    std::string cc = mnemonic.substr(2);
    // Iterate over all the possibilities, looking for a match
    // Value at index i corresponds to nzp i + 1
    std::vector<std::string> cc_possibilities = {"p",  "z",  "zp", "n",
                                                 "np", "nz", "nzp"};
    bool match = false;
    uint8_t nzp;
    for (uint8_t i = 0; i < cc_possibilities.size(); i++) {
      if (cc == cc_possibilities[i]) {
        match = true;
        nzp = i + 1;
      }
    }
    // If no match, die
    if (!match)
      return this->Error(
          NameLoc, "invalid mnemonic" +
                       LC32MnemonicSpellCheck(mnemonic,
                                              this->getAvailableFeatures(), 0));
    // Otherwise, add
    Operands.push_back(std::make_unique<LC32OperandToken>("br", NameLoc));
    Operands.push_back(std::make_unique<LC32OperandNZP>(nzp, NameLoc, NameLoc));

  } else {
    // Check that the mnemonic is correct
    // Then just add the mnemonic to the operands
    if (!LC32CheckMnemonic(mnemonic, this->getAvailableFeatures(), 0))
      return this->Error(
          NameLoc, "invalid mnemonic" +
                       LC32MnemonicSpellCheck(mnemonic,
                                              this->getAvailableFeatures(), 0));
    Operands.push_back(std::make_unique<LC32OperandToken>(mnemonic, NameLoc));
  }

  // Parse all the operands
  bool first_operand = true;
  while (!this->getLexer().is(AsmToken::EndOfStatement)) {
    // If this is not the first operand, expect and consume a comma
    if (!first_operand) {
      if (!this->getLexer().is(AsmToken::Comma)) {
        return this->Error(this->getLexer().getLoc(), "expected comma");
      }
      this->getLexer().Lex();
    }

    // Try to parse using all the parsers
    bool success = false;
    for (auto parser : OPERAND_PARSERS) {
      std::unique_ptr<LC32Operand> op;
      auto r = parser(*this, op);
      // If the parse didn't work, either die or try the next one
      if (r == MatchOperand_ParseFail) {
        success = false;
        break;
      } else if (r == MatchOperand_NoMatch) {
        continue;
      }
      // Otherwise, add
      // Also go to the next operand
      Operands.push_back(std::move(op));
      success = true;
      break;
    }

    // Check for success and die if not
    if (!success)
      return this->Error(this->getLexer().getLoc(), "could not parse operand");

    // Move on
    // Remember to set whether this is the first operand
    first_operand = false;
  }

  return false;
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

  // For an invalid operand, ErrorInfo contains the operand index containing
  // the error, or ~0ull if not present.
  case Match_InvalidOperand:
    if (ErrorInfo == ~0ull)
      return this->Error(IDLoc, "bad operand value");
    if (ErrorInfo >= Operands.size())
      return this->Error(IDLoc, "too many operands");
    return this->Error(
        static_cast<LC32Operand &>(*Operands[ErrorInfo]).getStartLoc(),
        "bad operand value");

  // If all else fails, return a generic failure
  default:
    return true;
  }
}
