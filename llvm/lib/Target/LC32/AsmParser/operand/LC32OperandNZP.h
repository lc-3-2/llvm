//===- LC32OperandNZP.h - Condition Code Operand for LC-3.2 ---------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines a class to hold condition code operands for BR. This
// doesn't have a parse method since it isn't parsed normally. Instead, it is
// part of the mnemonic.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDNZP_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDNZP_H

#include "LC32Operand.h"

namespace llvm {

class LC32OperandNZP : public LC32Operand {
public:
  LC32OperandNZP(uint8_t Value, SMLoc StartLoc, SMLoc EndLoc);
  void print(raw_ostream &OS) const override;

  bool isNZP() const override;

  void addNZPOperands(MCInst &Inst, unsigned N) override;

private:
  uint8_t Value;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32OPERANDNZP_H
