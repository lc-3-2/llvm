//===-- LC32MCTargetDesc.cpp - LC-3.2 Target Descriptions -----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32MCTargetDesc.h"
#include "TargetInfo/LC32TargetInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/TargetRegistry.h"
using namespace llvm;

#define GET_REGINFO_MC_DESC
#include "LC32GenRegisterInfo.inc"

#define GET_INSTRINFO_MC_DESC
#define ENABLE_INSTR_PREDICATE_VERIFIER
#include "LC32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "LC32GenSubtargetInfo.inc"

static MCInstrInfo *createLC32MCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitLC32MCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createLC32MCRegisterInfo(const Triple & /*TT*/) {
  // This method passes the register that stores the return address. It doesn't
  // seem to be used anywhere though.
  MCRegisterInfo *X = new MCRegisterInfo();
  InitLC32MCRegisterInfo(X, LC32::LR);
  return X;
}

static MCSubtargetInfo *createLC32MCSubtargetInfo(const Triple &TT,
                                                  StringRef CPU, StringRef FS) {
  // The Lanai target checks if the CPU name is empty. However, this method can
  // take an empty string as input. In that case, it will just apply all the
  // features given in `FS`.
  //
  // See: llvm/lib/MC/MCSubtargetInfo.cpp
  return createLC32MCSubtargetInfoImpl(TT, CPU, /*TuneCPU*/ CPU, FS);
}

static MCStreamer *createMCStreamer(const Triple &T, MCContext &Context,
                                    std::unique_ptr<MCAsmBackend> &&MAB,
                                    std::unique_ptr<MCObjectWriter> &&OW,
                                    std::unique_ptr<MCCodeEmitter> &&Emitter,
                                    bool RelaxAll) {
  // We're guaranteed to be using ELF. Also, we can use the generic ELF streamer
  // since we don't have special handling, unlike MSP430.
  return createELFStreamer(Context, std::move(MAB), std::move(OW),
                           std::move(Emitter), RelaxAll);
}

extern "C" LLVM_EXTERNAL_VISIBILITY void LLVMInitializeLC32TargetMC() {
  // Auto-discovered method
  // Associate all the components of the MC layer with the LC-3.2 target handle
  // Notice that we use the generic createMCStreamer method since we don't need
  // anything custom for writing ELF files.
  Target &T = getTheLC32Target();
  TargetRegistry::RegisterMCRegInfo(T, createLC32MCRegisterInfo);
  TargetRegistry::RegisterMCInstrInfo(T, createLC32MCInstrInfo);
  TargetRegistry::RegisterMCSubtargetInfo(T, createLC32MCSubtargetInfo);
  TargetRegistry::RegisterELFStreamer(T, createMCStreamer);
}
