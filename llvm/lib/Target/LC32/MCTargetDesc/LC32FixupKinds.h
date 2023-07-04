//===-- LC32FixupKinds.h - LC-3.2 Assembly Fixup Entries --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// When emitting machine code from assembly, we usually have to remember a label
// and patch it later once the address is known. This module encodes the
// different ways to do that. Some of them correspond to relocations, while
// others are only possible if the label is close enough.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32FIXUPKINDS_H
#define LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32FIXUPKINDS_H

#include "llvm/MC/MCFixup.h"

// Since these don't have a very descriptive name on their own, I'll put this in
// a namespace for the LC-3.2
namespace llvm::lc32 {

// Must match LC32AsmBackend.cpp
enum Fixups {
  // Fixup for JSR
  TFK_PCOffset11 = FirstTargetFixupKind,

  // Fixups for both variants of PCOffset9
  TFK_PCOffset9BR,
  TFK_PCOffset9LEA,

  // Mark the end of the array
  // Also compute the count
  LastTargetFixupKind,
  NumTargetFixupKinds = LastTargetFixupKind - FirstTargetFixupKind
};

} // namespace llvm::lc32

#endif // LLVM_LIB_TARGET_LC32_MCTARGETDESC_LC32FIXUPKINDS_H
