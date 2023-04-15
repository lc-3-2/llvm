//===--- LC32.h - Declare LC-3.2 Target Feature Support ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module declares the `TargetInfo` class for the LC-3.2. This is how clang
// knows about the target and how to generate bitcode for it. It also handles
// inline assembly.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_LIB_BASIC_TARGETS_LC32_H
#define LLVM_CLANG_LIB_BASIC_TARGETS_LC32_H

#include "clang/Basic/TargetInfo.h"
#include "llvm/TargetParser/Triple.h"

namespace clang::targets {

class LLVM_LIBRARY_VISIBILITY LC32TargetInfo : public TargetInfo {
public:
  LC32TargetInfo(const llvm::Triple &Triple, const TargetOptions &);

  void getTargetDefines(const LangOptions &Opts,
                        MacroBuilder &Builder) const override;

  ArrayRef<const char *> getGCCRegNames() const override;
  ArrayRef<GCCRegAlias> getGCCRegAliases() const override;

  ArrayRef<Builtin::Info> getTargetBuiltins() const override;
  BuiltinVaListKind getBuiltinVaListKind() const override;

  const char *getClobbers() const override;
  bool validateAsmConstraint(const char *&Name,
                             TargetInfo::ConstraintInfo &info) const override;
};

} // namespace clang::targets

#endif // LLVM_CLANG_LIB_BASIC_TARGETS_LC32_H
