//===- LC32OperandToken.h - Token Operand for LC-3.2 ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "operand/LC32OperandToken.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;
#define DEBUG_TYPE "LC32AsmParser"

LC32OperandToken::LC32OperandToken(std::string Name, SMLoc StartLoc)
    : LC32Operand(StartLoc, StartLoc), Name(Name) {}

void LC32OperandToken::print(raw_ostream &OS) const {
  OS << "LC32OperandToken " << this->Name;
}

bool LC32OperandToken::isToken() const { return true; }
StringRef LC32OperandToken::getToken() const { return this->Name; }
