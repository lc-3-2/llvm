//===- LC32OperandExpr.cpp - Immediate Operand for LC-3.2 -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "operand/LC32OperandExpr.h"
#include "LC32AsmParser.h"
#include "operand/LC32OperandImm.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmParser"

LC32OperandExpr::LC32OperandExpr(const MCExpr *Expr, SMLoc StartLoc,
                                 SMLoc EndLoc)
    : LC32Operand(StartLoc, EndLoc), Expr(Expr) {}

void LC32OperandExpr::print(raw_ostream &OS) const {
  OS << "LC32OperandExpr " << *this->Expr;
}

bool LC32OperandExpr::isImm() const { return true; }
bool LC32OperandExpr::isPCOffset9() const { return true; }
bool LC32OperandExpr::isPCOffset11() const { return true; }
bool LC32OperandExpr::isConst32() const { return true; }

void LC32OperandExpr::addPCOffset9Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createExpr(this->Expr));
}
void LC32OperandExpr::addPCOffset11Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createExpr(this->Expr));
}
void LC32OperandExpr::addConst32Operands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createExpr(this->Expr));
}

OperandMatchResultTy
llvm::lc32::OPERAND_PARSER_EXPR(LC32AsmParser &t,
                                std::unique_ptr<LC32Operand> &op) {

  // Remember the starting location
  SMLoc stt = t.getLexer().getLoc();

  // Try to parse an expression
  const MCExpr *expr;
  SMLoc end;
  if (t.getParser().parseExpression(expr, end))
    return MatchOperand_ParseFail;

  // Check if the expression evaluates to a constant
  // If it does, use an immediate
  int64_t expr_abs;
  if (expr->evaluateAsAbsolute(expr_abs)) {
    op = std::make_unique<LC32OperandImm>(expr_abs, stt, end);
    return MatchOperand_Success;
  }

  // Otherwise, construct an expression
  op = std::make_unique<LC32OperandExpr>(expr, stt, end);
  return MatchOperand_Success;
}
