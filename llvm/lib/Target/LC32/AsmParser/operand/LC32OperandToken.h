//===- LC32OperandToken.h - Token Operand for LC-3.2 ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a Token operand for the LC-3.2. It's just a string with a
// start location. Also, it just overrides the required methods from the
// superclass. Note that it takes a std::string instead of a StringRef.
// Logically, it owns the string.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDTOKEN_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDTOKEN_H

#include "LC32Operand.h"
#define DEBUG_TYPE "LC32AsmParser"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32OperandToken : public LC32Operand {
public:
  LC32OperandToken(std::string Name, SMLoc StartLoc);
  void print(raw_ostream &OS) const override;

  bool isToken() const override;
  StringRef getToken() const override;

private:
  std::string Name;
};

} // namespace llvm::lc32

#undef DEBUG_TYPE

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDTOKEN_H
