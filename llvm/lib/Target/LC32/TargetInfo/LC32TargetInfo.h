//===-- LC32TargetInfo.h - LC-3.2 Target Implementation ---------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This small module provides a handle to identify LC-3.2, registering it with
// the `TargetRegistry`. Other LC-3.2 components then register with the same
// handle.
//
// The `llvm::getTheLC32Target()` method returns the handle. It's a reference to
// a statically allocated `Target`.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_TARGETINFO_LC32TARGETINFO_H
#define LLVM_LIB_TARGET_LC32_TARGETINFO_LC32TARGETINFO_H

namespace llvm {
class Target;

Target &getTheLC32Target();

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_TARGETINFO_LC32TARGETINFO_H
