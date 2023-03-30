//===-- LC32MCTargetDesc.h - LC-3.2 Target Descriptions ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module provides access to the MC-layer for the LC-3.2. It registers all
// the components with the target registry, and this header provides definitions
// for registers and instructions.
//
// See: llvm/lib/Target/Lanai/MCTargetDesc/LanaiMCTargetDesc.*
// See: llvm/lib/Target/MSP430/MCTargetDesc/MSP430MCTargetDesc.*
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCTARGETDESC_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCTARGETDESC_H

#include "llvm/Support/DataTypes.h"

namespace llvm {
class Target;
class MCAsmBackend;
class MCCodeEmitter;
class MCInstrInfo;
class MCSubtargetInfo;
class MCRegisterInfo;
class MCContext;
class MCTargetOptions;
class MCObjectTargetWriter;
class MCStreamer;
class MCTargetStreamer;
} // namespace llvm

#define GET_REGINFO_ENUM
#include "LC32GenRegisterInfo.inc"

#define GET_INSTRINFO_ENUM
#define GET_INSTRINFO_MC_HELPER_DECLS
#include "LC32GenInstrInfo.inc"

#define GET_SUBTARGETINFO_ENUM
#include "LC32GenSubtargetInfo.inc"

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32MCTARGETDESC_H
