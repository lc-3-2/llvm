//===-- LC32DisassemblerRegistration.cpp - Register the Disassembler ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file handles registration for the disassembler. It's pulled out from the
// rest of the implementation just for organizational purposes. The code works
// in the same way as the `LC32TargetMachine`.
//
//===----------------------------------------------------------------------===//

#include "LC32Disassembler.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;
using namespace llvm::LC32;
#define DEBUG_TYPE "Disassembler"

static MCDisassembler *createLC32Disassembler(const Target &T,
                                              const MCSubtargetInfo &STI,
                                              MCContext &Ctx) {
  return new LC32Disassembler(STI, Ctx);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32Disassembler() {
  TargetRegistry::RegisterMCDisassembler(getTheLC32Target(),
                                         createLC32Disassembler);
}
