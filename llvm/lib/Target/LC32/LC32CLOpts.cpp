//==- LC32CLOpts.cpp - Target Command-Line Options for LC-3.2 ---*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32CLOpts.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32CLOpts"

static_assert(std::is_same<size_t, unsigned long>::value, "Bad type");
static_assert(std::is_same<ssize_t, long>::value, "Bad type");
static_assert(std::is_same<uint64_t, unsigned long>::value, "Bad type");
static_assert(std::is_same<int64_t, long>::value, "Bad type");

cl::opt<unsigned> llvm::lc32::clopts::MaxRepeatedOps(
    "lc_3.2-max-repeated-ops",
    cl::desc("How many instructions to use for repeated operations, such as "
             "repeated ADDs to realize constants, before giving up and using "
             "PSEUDO.LOADCONST* instead"),
    cl::init(4));
cl::opt<unsigned> llvm::lc32::clopts::MaxConstMulHammingWeight(
    "lc_3.2-max-const-mul-hamming-weight",
    cl::desc("When performing multiplications by constants, how many bits "
             "the constant can have before it is profitable to just libcall"),
    cl::init(3));

cl::opt<bool>
    llvm::lc32::clopts::UseR4("lc_3.2-use-r4",
                              cl::desc("Allow use of R4 during functions"),
                              cl::init(false));
cl::opt<bool>
    llvm::lc32::clopts::UseR7("lc_3.2-use-r7",
                              cl::desc("Allow use of R7 during functions"),
                              cl::init(false));

cl::opt<bool> llvm::lc32::clopts::EnableTestElision(
    "lc_3.2-enable-test-elision",
    cl::desc("Remove unneeded tests before branches"), cl::init(true),
    cl::Hidden);

cl::opt<bool> llvm::lc32::clopts::EnableRedundantClearElimination(
    "lc_3.2-enable-redundant-clear-elimination",
    cl::desc("Remove unneeded register clears after branches"), cl::init(true),
    cl::Hidden);
