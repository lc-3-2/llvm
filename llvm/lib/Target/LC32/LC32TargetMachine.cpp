//===-- LC32TargetMachine.cpp - Define TargetMachine for LC-3.2 -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32TargetMachine.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
#include <optional>
using namespace llvm;

static Reloc::Model getEffectiveRelocModel(std::optional<Reloc::Model> RM) {
  // Default relocation model
  return RM.value_or(Reloc::Static);
}

static std::string computeDataLayout(const Triple &TT, StringRef CPU,
                                     const TargetOptions &Options) {
  return "e"        // Little endian
         "-S32"     // Stack is word aligned
         "-p:32:32" // Pointers are words and are word aligned
         "-i32:32"  // Words are word aligned
         "-i16:16"  // Halfwords are halfword aligned
         "-i8:8"    // Bytes are byte aligned
         "-a:32"    // Aggregates are word aligned
         "-n32"     // Integers are 32-bit
         "-m:e"     // ELF-style name mangling
      ;
}

LC32TargetMachine::LC32TargetMachine(const Target &T, const Triple &TT,
                                     StringRef CPU, StringRef FS,
                                     const TargetOptions &Options,
                                     std::optional<Reloc::Model> RM,
                                     std::optional<CodeModel::Model> CM,
                                     CodeGenOpt::Level OL, bool JIT)
    : LLVMTargetMachine(T, computeDataLayout(TT, CPU, Options), TT, CPU, FS,
                        Options, getEffectiveRelocModel(RM),
                        getEffectiveCodeModel(CM, CodeModel::Small), OL) {
  // Initialize the MC layer
  // Componenents are retrieved automatically through the target registry
  this->initAsmInfo();
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32Target() {
  // Auto-discovered method
  // Associate the LC-3.2 target handle with its `TargetMachine`
  RegisterTargetMachine<LC32TargetMachine> X(getTheLC32Target());
}
