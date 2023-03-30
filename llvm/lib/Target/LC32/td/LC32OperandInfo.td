//===- LC32OperandInfo.td - Operand Definitions ------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains definitions for all the operand types for the LC-3.2.
// Here, it serves the dual purpose of parsing assembly and matching operands
// during code generation. Most of the definitions come in pairs - the operand
// itself and the operand class to use while parsing assembly files.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

//---
// Used by: ADD, AND, XOR

def Imm5OperandClass : AsmOperandClass {
  let Name = "Imm5";
}

def Imm5 : Operand<i32>, ImmLeaf<i32, [{ return isInt<5>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = Imm5OperandClass;
}
