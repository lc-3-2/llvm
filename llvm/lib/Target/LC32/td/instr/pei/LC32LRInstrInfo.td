//===- LC32LRInstrInfo.td - LR Management Instructions -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares internally used instructions for saving and restoring the
// link register. R7 is in a wierd place, where it can be used as a general
// purpose register but has an ABI-defined place where it is saved on the stack.
// LLVM doesn't like this since it expects callee save registers to be saved
// only in stack slots. This causes compilation to fail when both
// `--lc_3.2-use-r7` and `--verify-machineinstrs` are active.
//
// We solve this problem by hiding the store and restore of R7 behind these
// pseudo instructions. These are only ever used by the prolog/epilog insertion
// code in LC32FrameLowering.cpp, and they don't flag that they use LR.
//
// Strictly speaking, `C_LDLR` is not needed. It's perfectly fine to load a
// value and never use it in the epilog. We keep it in for consistency.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def C_STLR : LC32Instruction_C, PseudoInstExpansion<(STW LR, GPR:$baser, WOffset6:$woffset6)> {
  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$baser, WOffset6:$woffset6);

  let mayStore = true;
  let hasSideEffects = true;

  let Size = 2;
}

def C_LDLR : LC32Instruction_C, PseudoInstExpansion<(LDW LR, GPR:$baser, WOffset6:$woffset6)> {
  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$baser, WOffset6:$woffset6);

  let mayLoad = true;
  let hasSideEffects = true;

  let Size = 2;
}
