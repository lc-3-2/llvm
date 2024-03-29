//===- LC32Operand.h - Assembly Operands for LC-3.2 -----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines an abstract superclass for all the MCParsedAsmOperands used
// by the LC-3.2. Usually, this is defined as a single class in the
// AsmParser.cpp file, using enums and unions to discriminate between the types.
//
// This file also defines the type of a function that parses to operands, named
// `operand_parser_t`. Functions of this type should try to parse one type of
// operand, returning the status of the parse along with the parsed operand.
// These should be registered in the OperandParsers array in
// LC32AsmParserImpl.cpp.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERAND_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERAND_H

#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/Support/ErrorHandling.h"
#define DEBUG_TYPE "LC32AsmParser"

namespace llvm {

class LC32Operand : public MCParsedAsmOperand {
public:
  LC32Operand(SMLoc StartLoc, SMLoc EndLoc) : stt(StartLoc), end(EndLoc) {}

  // Common code for locations
  virtual SMLoc getStartLoc() const override { return this->stt; }
  virtual SMLoc getEndLoc() const override { return this->end; }

  // Type determination
  // Only isToken and isReg are used. Everything else throws.
  virtual bool isToken() const override { return false; }
  virtual bool isReg() const override { return false; }
  virtual bool isImm() const override {
    llvm_unreachable("Method should never be called");
  }
  virtual bool isMem() const override {
    llvm_unreachable("Method should never be called");
  }

  // Data getters
  // These throw by default. Subclasses should override if they have information
  // to provide.
  virtual StringRef getToken() const { llvm_unreachable("Bad type"); }
  virtual unsigned getReg() const override { llvm_unreachable("Bad type"); }

  // Query methods
  // Needed by TableGen, and specified in the TableGen files. Also, these can't
  // be made into a template because it's virtual.
  virtual bool isImm5() const { return false; }
  virtual bool isAmount3() const { return false; }
  virtual bool isBOffset6() const { return false; }
  virtual bool isHOffset6() const { return false; }
  virtual bool isWOffset6() const { return false; }
  virtual bool isPCOffset9BR() const { return false; }
  virtual bool isPCOffset9LEA() const { return false; }
  virtual bool isPCOffset11() const { return false; }
  virtual bool isTrapVect8() const { return false; }
  virtual bool isConst16() const { return false; }
  virtual bool isConst32() const { return false; }
  virtual bool isNZP() const { return false; }

  // Render methods
  // Needed by TableGen, and specified in the TableGen files. Also, these can't
  // be made into a template because it's virtual.
  virtual void addRegOperands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addImm5Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addAmount3Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addBOffset6Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addHOffset6Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addWOffset6Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addPCOffset9BROperands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addPCOffset9LEAOperands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addPCOffset11Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addTrapVect8Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addConst16Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addConst32Operands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }
  virtual void addNZPOperands(MCInst &Inst, unsigned N) {
    llvm_unreachable("Bad type");
  }

private:
  // This anchor is placed in `LC32AsmParserImpl.cpp`
  virtual void anchor();

  // See getters
  SMLoc stt;
  SMLoc end;
};

class LC32AsmParser;
/**
 * \brief Operand parser function signature
 *
 * Functions using this signature should try to parse one type of operand. The
 * parsed operand should be returned in `op` if successful.
 *
 * If unsuccessful, it can be so in two ways. It can fail with
 * `MatchOperand_NoMatch`, meaning the operand was not recognized as an element
 * of the class being parsed. It could be of another class - like it could be an
 * immediate instead of a register. Thus, other operand parsers will be tried if
 * this status is returned. In this case, the function should not have touched
 * the lexer's state when it returns.
 *
 * Alternatively, the function can return `MatchOperand_ParseFail`. This means
 * the operand was recognized as an element of the class, but it is not
 * syntactically correct. For instance, writing R9. No other operand parsers
 * will be tried after this status is returned, and parsing will fail
 * unconditionally.
 *
 * \see LC32AsmParser::ParseInstruction
 *
 * \param [in]  t reference to the calling LC32AsmParser
 * \param [out] op the parsed operand
 * \return result of the parse
 */
typedef OperandMatchResultTy operand_parser_t(LC32AsmParser &t,
                                              std::unique_ptr<LC32Operand> &op);

} // namespace llvm

#undef DEBUG_TYPE

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERAND_H
