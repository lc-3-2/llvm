//===- LC32MoveInstrInfo.td - Move Psuedo Instructions -----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains instructions to move register and immediate values. These
// are all pseudo instructions that are lowered at some point, usually in the
// AsmPrinter. Everything with PseudoInstExpansion will be auto-generated, while
// everything else has to be manually lowered.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def C_MOVE_REG : Instruction, PseudoInstExpansion<(ADDi GPR:$dr, GPR:$sr, 0)> {
  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$sr);

  let Pattern = [(set GPR:$dr, GPR:$sr)];

  let isMoveReg = true;
  let isAsCheapAsAMove = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

//---

def C_MOVE_ZERO : Instruction, PseudoInstExpansion<(ANDi GPR:$dr, GPR:$dr, 0)> {
  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins);

  let Pattern = [(set GPR:$dr, 0)];

  let isMoveImm = true;
  let isAsCheapAsAMove = true;
  let isReMaterializable = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

def C_MOVE_IMM5 : Instruction {
  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins Imm5:$imm5);

  let Pattern = [(set GPR:$dr, Imm5:$imm5)];

  let isMoveImm = true;
  let isAsCheapAsAMove = true;
  let isReMaterializable = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

//---
