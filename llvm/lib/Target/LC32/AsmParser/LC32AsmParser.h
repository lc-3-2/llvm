//===- LC32AsmParser.h - Parse LC-3.2 Assembly to MCInst ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module exists to parse assembly files to MCInsts. Most of the heavy
// lifting is done by TableGen, but we're still responsible for parsing
// individual operands and labeling them with types. That information is used by
// the matcher to select the appropriate instruction.
//
// See: LC32GenAsmMatcher.inc
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H

#include "llvm/MC/MCParser/MCTargetAsmParser.h"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::lc32 {

class LC32AsmParser : public MCTargetAsmParser {
public:
  LC32AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options);

  // Useful methods, both for us and for other classes that parse operands
  MCAsmParser &getParser() const;
  MCAsmLexer &getLexer() const;

  // Register parsing
  // Needed to make class concrete
  OperandMatchResultTy tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;

private:
  // The parser very useful for this class, so keep it
  MCAsmParser &Parser;
  // These variables are also used by some methods, so keep them around
  const MCSubtargetInfo &STI;

  // Register parsing
  // Needed to make class concrete
  bool parseRegister(MCRegister &Reg, SMLoc &StartLoc, SMLoc &EndLoc) override;

  // Instruction parsing and matching
  // Needed to make class concrete
  bool ParseDirective(AsmToken DirectiveID) override;
  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;
  bool MatchAndEmitInstruction(SMLoc IDLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

// Provides: ComputeAvailableFeatures
#define GET_ASSEMBLER_HEADER
#include "LC32GenAsmMatcher.inc"
};

} // namespace llvm::lc32

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H
