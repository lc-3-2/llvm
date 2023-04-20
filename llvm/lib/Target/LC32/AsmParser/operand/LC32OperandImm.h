//===- LC32OperandImm.h - Immediate Operand for LC-3.2 --------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines immediate operands for the LC-3.2. Mainly, it is a wrapper
// class that stores the immedaite's value. This file also provides a parser
// function for immediates.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDIMM_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDIMM_H

#include "LC32Operand.h"
#define DEBUG_TYPE "LC32AsmParser"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32OperandImm : public LC32Operand {
public:
  LC32OperandImm(int64_t Value, SMLoc StartLoc, SMLoc EndLoc);
  void print(raw_ostream &OS) const override;

  bool isImm() const override;
  bool isImm5() const override;
  bool isAmount3() const override;
  bool isTrapVect8() const override;

  void addImm5Operands(MCInst &Inst, unsigned N) override;
  void addAmount3Operands(MCInst &Inst, unsigned N) override;
  void addTrapVect8Operands(MCInst &Inst, unsigned N) override;

private:
  uint64_t Value;
};

extern operand_parser_t OPERAND_PARSER_IMM;

} // namespace llvm::lc32

#undef DEBUG_TYPE

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDIMM_H
