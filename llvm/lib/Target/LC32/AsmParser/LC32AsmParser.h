//===- LC32AsmParser.h - Parse LC-3.2 Assembly to MCInst ------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H
#define LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H

#include "llvm/MC/MCParser/MCTargetAsmParser.h"

// Usually, the contents of this file are in an anonymous namespace. I'll put
// them in a namespace for the LC-3.2
namespace llvm::LC32 {

class LC32AsmParser : public MCTargetAsmParser {
public:
  LC32AsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                const MCInstrInfo &MII, const MCTargetOptions &Options);

private:
  // The parser very useful for this class, so keep it
  MCAsmParser &Parser;
  // These variables are also used by some methods, so keep them around
  const MCSubtargetInfo &STI;

  // Useful methods
  MCAsmParser &getParser() const;
  MCAsmLexer &getLexer() const;

  // Register parsing
  // Needed to make class concrete
  OperandMatchResultTy tryParseRegister(MCRegister &Reg, SMLoc &StartLoc,
                                        SMLoc &EndLoc) override;
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

} // namespace llvm::LC32

#endif // LLVM_LIB_TARGET_LC32_ASMPARSER_LC32ASMPARSER_H
