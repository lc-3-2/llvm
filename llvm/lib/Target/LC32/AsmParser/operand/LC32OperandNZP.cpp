//===- LC32OperandNZP.cpp - Condition Code Operand for LC-3.2 -------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "operand/LC32OperandNZP.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmParser"

LC32OperandNZP::LC32OperandNZP(uint8_t Value, SMLoc StartLoc, SMLoc EndLoc)
    : LC32Operand(StartLoc, EndLoc), Value(Value) {}

void LC32OperandNZP::print(raw_ostream &OS) const {
  OS << "LC32OperandImm " << static_cast<int>(this->Value);
}

bool LC32OperandNZP::isNZP() const { return true; }

void LC32OperandNZP::addNZPOperands(MCInst &Inst, unsigned N) {
  assert(N == 1 && "Invalid number of operands!");
  Inst.addOperand(MCOperand::createImm(this->Value));
}
