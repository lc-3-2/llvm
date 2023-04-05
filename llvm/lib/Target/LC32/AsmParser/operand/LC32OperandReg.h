//===- LC32OperandReg.h - Register Operand for LC-3.2 ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines register operands for the LC-3.2. Mainly, it is a wrapper
// class that stores the internal register number. This file also provides a
// parser function for registers.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDREG_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDREG_H

#include "LC32Operand.h"
#define DEBUG_TYPE "LC32AsmParser"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32OperandReg : public LC32Operand {
public:
  LC32OperandReg(unsigned Number, SMLoc StartLoc, SMLoc EndLoc);
  void print(raw_ostream &OS) const override;

  bool isReg() const override;
  unsigned getReg() const override;

  void addRegOperands(MCInst &Inst, unsigned N) override;

private:
  unsigned Number;
};

extern operand_parser_t OPERAND_PARSER_REG;

} // namespace llvm::lc32

#undef DEBUG_TYPE

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDREG_H
