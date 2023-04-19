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
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

include "td/LC32OperandInfo.td"

include "td/instr/LC32ALUInstrInfo.td"
include "td/instr/LC32SupervisorInstrInfo.td"

def LC32InstrInfo : InstrInfo;
