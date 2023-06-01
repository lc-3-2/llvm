//===- LC32OperandImm.h - Immediate Operand for LC-3.2 --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines expression operands for the LC-3.2. This is for anything
// that can hold a symbol reference - PCOffset9 and PCOffset11. It also encodes
// for Const32 since that can have symbols. As always, it provides a parser
// function too.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDEXPR_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDEXPR_H

#include "LC32Operand.h"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32OperandExpr : public LC32Operand {
public:
  LC32OperandExpr(const MCExpr *Expr, SMLoc StartLoc, SMLoc EndLoc);
  void print(raw_ostream &OS) const override;

  bool isImm() const override;
  bool isPCOffset9BR() const override;
  bool isPCOffset9LEA() const override;
  bool isPCOffset11() const override;
  bool isConst32() const override;

  void addPCOffset9BROperands(MCInst &Inst, unsigned N) override;
  void addPCOffset9LEAOperands(MCInst &Inst, unsigned N) override;
  void addPCOffset11Operands(MCInst &Inst, unsigned N) override;
  void addConst32Operands(MCInst &Inst, unsigned N) override;

private:
  const MCExpr *Expr;
};

extern operand_parser_t OPERAND_PARSER_EXPR;

} // namespace llvm::lc32

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDIMM_H
