//===- LC32OperandImm.cpp - Immediate Operand for LC-3.2 ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "operand/LC32OperandImm.h"
#include "LC32AsmParser.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmParser"

LC32OperandImm::LC32OperandImm(int64_t Value, SMLoc StartLoc, SMLoc EndLoc)
    : LC32Operand(StartLoc, EndLoc), Value(Value) {}

void LC32OperandImm::print(raw_ostream &OS) const {
  OS << "LC32OperandImm " << this->Value;
}

bool LC32OperandImm::isImm() const { return true; }
bool LC32OperandImm::isImm5() const { return isInt<5>(this->Value); }
bool LC32OperandImm::isAmount3() const { return isUInt<3>(this->Value); }
bool LC32OperandImm::isBOffset6() const { return isInt<6>(this->Value); }
bool LC32OperandImm::isHOffset6() const { return isInt<6>(this->Value); }
bool LC32OperandImm::isWOffset6() const { return isInt<6>(this->Value); }
bool LC32OperandImm::isTrapVect8() const { return isUInt<8>(this->Value); }

void LC32OperandImm::addImm5Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value));
}
void LC32OperandImm::addAmount3Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value));
}
void LC32OperandImm::addBOffset6Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value << 0));
}
void LC32OperandImm::addHOffset6Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value << 1));
}
void LC32OperandImm::addWOffset6Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value << 2));
}
void LC32OperandImm::addTrapVect8Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value));
}

OperandMatchResultTy
llvm::lc32::OPERAND_PARSER_IMM(LC32AsmParser &t,
                               std::unique_ptr<LC32Operand> &op) {

  // Remember the starting location
  SMLoc stt = t.getLexer().getLoc();

  // If the next token is a hash, we're committed
  bool committed = false;
  if (t.getLexer().is(AsmToken::Hash)) {
    committed = true;
    t.getLexer().Lex();
  }

  // Parse the sign
  // If there is a sign, it commits us too
  int64_t sign = 1;
  if (t.getLexer().is(AsmToken::Minus)) {
    sign = -1;
    committed = true;
    t.getLexer().Lex();
  }

  // Parse integers
  // Special case for hex integers
  // They're parsed as identifiers, so we have to parse them ourselves
  if (t.getLexer().is(AsmToken::Integer)) {
    // Parse the value
    // Remember to track the end location
    int64_t val = sign * t.getLexer().getTok().getIntVal();
    SMLoc end = t.getLexer().getTok().getEndLoc();
    t.getLexer().Lex();
    // Create and return
    op = std::make_unique<LC32OperandImm>(val, stt, end);
    return MatchOperand_Success;

  } else if (t.getLexer().is(AsmToken::Identifier)) {
    // Check that it starts with an x
    StringRef id = t.getLexer().getTok().getIdentifier();
    if (id.startswith("x") || id.startswith("X")) {
      // It should have at least one digit after it
      if (id.size() < 2)
        return MatchOperand_ParseFail;
      // Parse the rest of the digits
      int64_t val = 0;
      for (size_t i = 1; i < id.size(); i++) {
        // If too long, die
        if (i > 8)
          return MatchOperand_ParseFail;
        // Convert the current character
        char c = id[i];
        int64_t c_val;
        if ('0' <= c && c <= '9')
          c_val = c - '0';
        else if ('A' <= c && c <= 'F')
          c_val = c - 'A' + 10;
        else if ('a' <= c && c <= 'f')
          c_val = c - 'a' + 10;
        else
          return MatchOperand_ParseFail;
        // Add to the value
        val <<= 4;
        val += c_val;
      }
      // Handle sign
      val *= sign;
      // Same as with normal integers
      SMLoc end = t.getLexer().getTok().getEndLoc();
      t.getLexer().Lex();
      op = std::make_unique<LC32OperandImm>(val, stt, end);
      return MatchOperand_Success;
    }
  }

  // We failed, so die
  return committed ? MatchOperand_ParseFail : MatchOperand_NoMatch;
}
