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
// Note the use of AddedComplexity. These are used to prioritize which
// instructions should be selected when moving an immediate into a register.
// They were chosen by looking at LC32GenDAGISel.inc.
//
// See: LC32MoveInstrInfo.td
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

let
  OutOperandList = (outs GPR:$dr),
  isMoveImm = true,
  isReMaterializable = true
in {

  def P_LOADCONSTH : LC32Instruction_P {
    let AsmString = "PSEUDO.LOADCONSTH\t$dr, $const16";

    let InOperandList = (ins Const16:$const16);
    let Pattern = [(set GPR:$dr, Const16:$const16)];
    let AddedComplexity = 1;
  }

  def P_LOADCONSTW : LC32Instruction_P {
    let AsmString = "PSEUDO.LOADCONSTW\t$dr, $const32";

    let InOperandList = (ins Const32:$const32);
    let Pattern = [(set GPR:$dr, Const32:$const32)];
    let AddedComplexity = 0;
  }
}

def P_LOADCONSTB : InstAlias<
  "PSEUDO.LOADCONSTB\t$dr, $const8",
  (P_LOADCONSTH GPR:$dr, Const16:$const8),
  0
>;
