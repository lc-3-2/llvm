//===--- LC32.cpp - Implement LC-3.2 Target Feature Support -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32.h"
#include "clang/Basic/MacroBuilder.h"

using namespace clang;
using namespace clang::targets;

LC32TargetInfo::LC32TargetInfo(const llvm::Triple &Triple,
                               const TargetOptions &)
    : TargetInfo(Triple) {
  // Set all the parameters
  // These are defaulted in the superclass constructor, so look at that to see
  // the values.
  // Note that `long` is 32-bit

  this->TLSSupported = false;
  this->VLASupported = false;
  this->HasLongDouble = false;

  // Set the data layout
  this->resetDataLayout(
      "e"        // Little endian
      "-S32"     // Stack is word aligned
      "-p:32:32" // Pointers are words and are word aligned
      "-i32:32"  // Words are word aligned
      "-i16:16"  // Halfwords are halfword aligned
      "-i8:8"    // Bytes are byte aligned
      "-a:8:32"  // Aggregates have to be byte-aligned, but prefer word aligned
      "-n32"     // Integers are 32-bit
      "-m:e"     // ELF-style name mangling
  );
}

void LC32TargetInfo::getTargetDefines(const LangOptions &Opts,
                                      MacroBuilder &Builder) const {
  // See: MSP430.cpp
  Builder.defineMacro("__LC_3_2__");
  Builder.defineMacro("__ELF__");
}

ArrayRef<const char *> LC32TargetInfo::getGCCRegNames() const {
  static const char *GCC_REG_NAMES[] = {"R0", "R1", "R2", "R3",
                                        "R4", "R5", "R6", "R7"};
  return ArrayRef<const char *>(GCC_REG_NAMES);
}

ArrayRef<TargetInfo::GCCRegAlias> LC32TargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCC_REG_ALIASES[] = {
      {{"AR"}, "R0"}, {{"XR"}, "R1"}, {{"YR"}, "R2"}, {{"AT"}, "R3"},
      {{"GP"}, "R4"}, {{"FP"}, "R5"}, {{"SP"}, "R6"}, {{"LR"}, "R7"}};
  return ArrayRef<TargetInfo::GCCRegAlias>(GCC_REG_ALIASES);
}

ArrayRef<Builtin::Info> LC32TargetInfo::getTargetBuiltins() const {
  return std::nullopt;
}

TargetInfo::BuiltinVaListKind LC32TargetInfo::getBuiltinVaListKind() const {
  return TargetInfo::VoidPtrBuiltinVaList;
}

const char *LC32TargetInfo::getClobbers() const { return ""; }

bool LC32TargetInfo::validateAsmConstraint(
    const char *&Name, TargetInfo::ConstraintInfo &info) const {
  return false;
}
