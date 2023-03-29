//===-- LC32TargetMachine.h - Define TargetMachine for LC-3.2 ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module creates and registers an `LLVMTargetMachine` for the LC-3.2. It
// provides high-level architectural details needed for machine code generation.
// It also provides an interface to query MC-layer details, and provides a way
// to access subtargets.
//
// Importantly, the class's constructor calls `LLVMTargetMachine::initAsmInfo()`
// to get all the MC-layer objects.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32TARGETMACHINE_H
#define LLVM_LIB_TARGET_LC32_LC32TARGETMACHINE_H

#include "llvm/Target/TargetMachine.h"
#include <optional>

namespace llvm {
class StringRef;

class LC32TargetMachine : public LLVMTargetMachine {
public:
  LC32TargetMachine(const Target &T, const Triple &TT, StringRef CPU,
                    StringRef FS, const TargetOptions &Options,
                    std::optional<Reloc::Model> RM,
                    std::optional<CodeModel::Model> CM, CodeGenOpt::Level OL,
                    bool JIT);
};
} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32TARGETMACHINE_H
