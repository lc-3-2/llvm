//===-- LC32AsmPrinter.cpp - LC-3.2 LLVM Assembly Writer ------------------===//
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

#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;
#define DEBUG_TYPE "LC32AsmPrinter"

namespace {

class LC32AsmPrinter : public AsmPrinter {
public:
  explicit LC32AsmPrinter(TargetMachine &TM,
                          std::unique_ptr<MCStreamer> Streamer);
};

} // namespace

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32AsmPrinter() {
  RegisterAsmPrinter<LC32AsmPrinter> X(getTheLC32Target());
}

LC32AsmPrinter::LC32AsmPrinter(TargetMachine &TM,
                               std::unique_ptr<MCStreamer> Streamer)
    : AsmPrinter(TM, std::move(Streamer)) {}
