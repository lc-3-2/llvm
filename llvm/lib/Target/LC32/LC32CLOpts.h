//==- LC32CLOpts.h - Target Command-Line Options for LC-3.2 -----*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module defines the command-line options used by other modules. It's
// essentially a bunch of cl::opt definitions.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32CLOPTS_H
#define LLVM_LIB_TARGET_LC32_LC32CLOPTS_H

#include "llvm/Support/CommandLine.h"

// Usually, options are declared as `static` in C++ files. I'll put them in
// their own namespace.
namespace llvm::lc32::clopts {

extern llvm::cl::opt<bool> UnsafeCMP;

extern llvm::cl::opt<unsigned> MaxRepeatedOps;
extern llvm::cl::opt<unsigned> MaxConstMulHammingWeight;

extern llvm::cl::opt<bool> UseR4;
extern llvm::cl::opt<bool> UseR7;

extern llvm::cl::opt<bool> EnableTestElision;
extern llvm::cl::opt<bool> EnableRedundantClearElimination;

} // namespace llvm::lc32::clopts

#endif // LLVM_LIB_TARGET_LC32_LC32CLOPTS_H
