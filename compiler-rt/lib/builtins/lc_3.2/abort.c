//===-- abort.c - Implement default abort ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This function is called when an unreachable state is reached from the
// runtime. By default, it calls __builtin_trap(), which the instruction
// selector makes TRAP 0xff - CRASH. However, the symbol is weak for
// implementations to override it.
//
//===----------------------------------------------------------------------===//

__attribute__((weak, noreturn)) void abort(void) {
  __builtin_trap();
}
