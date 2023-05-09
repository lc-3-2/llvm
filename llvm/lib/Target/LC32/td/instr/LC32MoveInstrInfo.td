//===- LC32MoveInstrInfo.td - Move Psuedo Instructions -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains instructions to move register and small immediate values.
// These are all pseudo instructions that are lowered at some point, usually in
// the AsmPrinter. Everything with PseudoInstExpansion will be auto-generated,
// while everything else has to be manually lowered.
//
// Note the use of AddedComplexity. These are used to prioritize which
// instructions should be selected when moving an immediate into a register.
// They were chosen by looking at LC32GenDAGISel.inc.
//
// See: LC32LoadConstInstrInfo.td
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

let
  OutOperandList = (outs GPR:$dr),
  isMoveImm = true,
  isAsCheapAsAMove = true,
  isReMaterializable = true
in {

  def C_MOVE_ZERO : LC32Instruction_C, PseudoInstExpansion<(ANDi GPR:$dr, GPR:$dr, 0)> {
    let InOperandList = (ins);
    let Pattern = [(set GPR:$dr, 0)];
    let AddedComplexity = 2;
  }

  def C_MOVE_IMM5 : LC32Instruction_C {
    let InOperandList = (ins Imm5:$imm5);
    let Pattern = [(set GPR:$dr, Imm5:$imm5)];
    let AddedComplexity = 2;
  }
}
