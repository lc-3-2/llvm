//===- LC32OperandReg.cpp - Register Operand for LC-3.2 -------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "operand/LC32OperandReg.h"
#include "LC32AsmParser.h"
#include "llvm/MC/MCRegister.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
#define DEBUG_TYPE "LC32AsmParser"

LC32OperandReg::LC32OperandReg(unsigned Number, SMLoc StartLoc, SMLoc EndLoc)
    : LC32Operand(StartLoc, EndLoc), Number(Number) {}

void LC32OperandReg::print(raw_ostream &OS) const {
  OS << "LC32OperandReg " << this->Number;
}

bool LC32OperandReg::isReg() const { return true; }
unsigned LC32OperandReg::getReg() const { return this->Number; }

void LC32OperandReg::addRegOperands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createReg(this->Number));
}

OperandMatchResultTy LC32OperandReg::parser(LC32AsmParser &t,
                                            std::unique_ptr<LC32Operand> &op) {
  // This just wraps around the functionality in the parser
  MCRegister n;
  SMLoc stt;
  SMLoc end;
  auto ret = t.tryParseRegister(n, stt, end);
  // Try to construct
  if (ret == MatchOperand_Success) {
    op = std::make_unique<LC32OperandReg>(n, stt, end);
  }
  // Return
  return ret;
}
