//===- LC32InstrInfo.td - Instruction Definitions ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions for all the instructions on the LC-3.2. It
// serves a dual purpose of defining the encoding for code emission and defining
// the semantics for instruction selection. This is usually the most complicated
// part of the tablegen, so here it's broken up into multiple files.
//
// The information here supports both the assembly and the code generation.
// Instructions prefixed with `P_` are pseudo instructions that disassemble to
// multiple instructions. Instructions prefixed with `C_` are for code
// generation. Additionally, declarations starting with `N_` are SelectionDAG
// nodes. Note that pseudo instructions still have sizes. These have to be
// overestimates of their actual size.
//
// SelectionDAG nodes need to be kept in sync with the declarations in
// LC32ISelLowering.h.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

include "td/LC32OperandInfo.td"

//---
// Common classes of instructions

class LC32Instruction : Instruction {
  let Namespace = "LC32";
  let Size = 2;

  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;

  bits<16> Inst;
}

class LC32Instruction_P : Instruction {
  let Namespace = "LC32";
  let isPseudo = true;

  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

class LC32Instruction_C : LC32Instruction_P {
  let isCodeGenOnly = true;
}

//---
// The order here is important for dependencies

include "td/instr/LC32LoadConstInstrInfo.td"

include "td/instr/LC32ALUInstrInfo.td"
include "td/instr/LC32SHFInstrInfo.td"
include "td/instr/LC32MoveInstrInfo.td"

include "td/instr/LC32MemInstrInfo.td"
include "td/instr/LC32SubroutineInstrInfo.td"
include "td/instr/LC32ConditionalInstrInfo.td"

include "td/instr/LC32SupervisorInstrInfo.td"

def LC32InstrInfo : InstrInfo;
