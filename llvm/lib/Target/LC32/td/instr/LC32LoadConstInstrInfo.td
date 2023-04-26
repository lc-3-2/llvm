//===- LC32LoadConstInstrInfo.td - Move Psuedo Instructions *- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains instructions to load big immediate values. These are all
// pseudo instructions, but the assembly code needs to know about them for
// relaxation. These have special code in the MCCodeEmitter.
//
// See: LC32MoveInstrInfo.td
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def P_LOADCONSTH : Instruction {
  let Namespace = "LC32";
  let AsmString = "PSEUDO.LOADCONSTH\t$dr, $const16";
  let isPseudo = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins Const16:$const16);

  let Pattern = [(set GPR:$dr, Const16:$const16)];

  let isMoveImm = true;
  let isReMaterializable = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

def P_LOADCONSTW : Instruction {
  let Namespace = "LC32";
  let AsmString = "PSEUDO.LOADCONSTW\t$dr, $const32";
  let isPseudo = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins Const32:$const32);

  let Pattern = [(set GPR:$dr, Const32:$const32)];

  let isMoveImm = true;
  let isReMaterializable = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

def P_LOADCONSTB : InstAlias<
  "PSEUDO.LOADCONSTB\t$dr, $const8",
  (P_LOADCONSTH GPR:$dr, Const16:$const8),
  0
>;
