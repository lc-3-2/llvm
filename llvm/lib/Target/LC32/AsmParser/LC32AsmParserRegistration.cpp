//===- LC32AsmParserRegistration.cpp - Register the Parser ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file handles registration for the assembler. It's pulled out from the
// rest of the implementation just for organizational purposes. The code works
// in the same way as the `LC32TargetMachine`.
//
//===----------------------------------------------------------------------===//

#include "LC32AsmParser.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;
using namespace llvm::lc32;
#define DEBUG_TYPE "LC32AsmParser"

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32AsmParser() {
  RegisterMCAsmParser<LC32AsmParser> X(getTheLC32Target());
}
