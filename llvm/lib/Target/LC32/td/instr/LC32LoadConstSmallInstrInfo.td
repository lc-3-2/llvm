//===- LC32LoadConstSmallInstrInfo.td - Move Instructions --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains instructions to move small immediate values. These are all
// pseudo instructions that are lowered at some point, usually in the
// AsmPrinter. Everything with PseudoInstExpansion will be auto-generated, while
// everything else has to be manually lowered.
//
// Note the use of AddedComplexity. These are used to prioritize which
// instructions should be selected when moving an immediate into a register.
// They were chosen by looking at LC32GenDAGISel.inc. However, these complexity
// values are not used anymore since this instruction is selected directly by
// LC32ISelDAGToDAG.cpp#LC32DAGToDAGISel::SelectRepeatedAdd.
//
// See: LC32LoadConstInstrInfo.td
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def C_LOADZERO : LC32Instruction_C, PseudoInstExpansion<(ANDi GPR:$dr, GPR:$dr, 0)> {
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins);
  let Pattern = [(set GPR:$dr, 0)];
  let AddedComplexity = 2;

  let isMoveImm = true;
  let isAsCheapAsAMove = true;
  let isReMaterializable = true;

  let Size = 2;
}

// If a constant is small enough, do either repeated xor-shifts or repeated
// additions to select it. Unfortunately, there doesn't seem to be a way to tell
// the matcher to do this automatically, even through ComplexPatterns. Thus, we
// do it in `Select` of `ISelDAGToDAG`.
