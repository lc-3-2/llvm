//===-- LC32TargetMachine.cpp - Define TargetMachine for LC-3.2 -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32TargetMachine.h"
#include "LC32ISelDAGToDAG.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/MC/TargetRegistry.h"
#include <optional>
using namespace llvm;
#define DEBUG_TYPE "LC32TargetMachine"

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
                        getEffectiveCodeModel(CM, CodeModel::Small), OL),
      TLOF(std::make_unique<TargetLoweringObjectFileELF>()),
      Subtarget(TT, std::string(CPU), std::string(FS), *this) {
  // Initialize the MC layer
  // Componenents are retrieved automatically through the target registry
  this->initAsmInfo();
}

TargetLoweringObjectFile *LC32TargetMachine::getObjFileLowering() const {
  return this->TLOF.get();
}

const LC32Subtarget *
LC32TargetMachine::getSubtargetImpl(const Function &F) const {
  return &Subtarget;
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32Target() {
  // Auto-discovered method
  // Associate the LC-3.2 target handle with its `TargetMachine`
  RegisterTargetMachine<LC32TargetMachine> X(getTheLC32Target());
}

namespace {

// This is where we add the passes for the code generator. The only required
// function here is to add the instruction selector.
class LC32PassConfig : public TargetPassConfig {
public:
  LC32PassConfig(LC32TargetMachine &TM, PassManagerBase &PM)
      : TargetPassConfig(TM, PM) {}
  bool addInstSelector() override;
};

} // namespace

TargetPassConfig *LC32TargetMachine::createPassConfig(PassManagerBase &PM) {
  return new LC32PassConfig(*this, PM);
}

bool LC32PassConfig::addInstSelector() {
  this->addPass(
      createLC32ISelDag(this->getTM<LC32TargetMachine>(), this->getOptLevel()));
  return false;
}
