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
  // Note that `long` is 32-bit and that `long double` is 64-bit
  this->TLSSupported = false;

  // Alignments above 32-bit don't mean anything on this target, and in fact
  // they cause errors since we can't realign the stack. Therefore, lower the
  // alignment of wide types.
  this->LongLongAlign = 32;
  this->Int128Align = 32;
  this->LongAccumAlign = 32;
  this->DoubleAlign = 32;
  this->LongDoubleAlign = 32;
  this->Float128Align = 32;
  this->SuitableAlign = 32;
  this->DefaultAlignForAttributeAligned = 32;

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
  // The names given here should match the defs given in LC32RegisterInfo.td.
  // Note that it's the def that has to match, not the name.
  static const char *GCC_REG_NAMES[] = {"R0", "R1", "R2", "AT",
                                        "GP", "FP", "SP", "LR"};
  return ArrayRef<const char *>(GCC_REG_NAMES);
}

ArrayRef<TargetInfo::GCCRegAlias> LC32TargetInfo::getGCCRegAliases() const {
  static const TargetInfo::GCCRegAlias GCC_REG_ALIASES[] = {
      {{"AR", "r0", "ar"}, "R0"},
      {{"XR", "r1", "xr"}, "R1"},
      {{"YR", "r2", "yr"}, "R2"},
      {{"R3", "r3", "at"}, "AT"},
      {{"R4", "r4", "gp"}, "GP"},
      {{"R5", "r5", "fp"}, "FP"},
      {{"R6", "r6", "sp"}, "SP"},
      {{"R7", "r7", "lr"}, "LR"}};
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
  // See: AVR.h

  // It seems like only one-character constraints are allowed
  if (StringRef(Name).size() > 1)
    return false;

  switch (*Name) {
  default:
    return false;
  case 'r': // Any register
  case 'a': // R0
  case 'x': // R1
  case 'y': // R2
  case 't': // R3
    info.setAllowsRegister();
    return true;
  case 'I': // imm5
    info.setRequiresImmediate(-16, 15);
    return true;
  case 'N': // amount5
    info.setRequiresImmediate(0, 31);
    return true;
  case 'O': // offset6
    info.setRequiresImmediate(-32, 31);
    return true;
  }
}
