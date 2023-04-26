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
// Some properties are useful for custom encoding and decoding of instructions.
// Specifically:
// * PrintMethod   for InstPrinter
// * EncoderMehtod for MCCodeEmitter
// * DecoderMethod for Disassembler
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
  let PrintMethod = "printShiftedSignedImmOperand<5,0>";
  let EncoderMethod = "getShiftedSignedImmOpValue<5,0>";
  let DecoderMethod = "DecodeShiftedSignedImm<5,0>";
}

//---
// Used by: SHF

def Amount3OperandClass : AsmOperandClass {
  let Name = "Amount3";
}

def Amount3 : Operand<i32>, ImmLeaf<i32, [{ return isUInt<3>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = Amount3OperandClass;
}

//---
// Used by: LD*, ST*

def BOffset6OperandClass : AsmOperandClass {
  let Name = "BOffset6";
}
def BOffset6 : Operand<i32>, ImmLeaf<i32, [{ return isShiftedInt<6,0>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = BOffset6OperandClass;
  let PrintMethod = "printShiftedSignedImmOperand<6,0>";
  let EncoderMethod = "getShiftedSignedImmOpValue<6,0>";
  let DecoderMethod = "DecodeShiftedSignedImm<6,0>";
}

def HOffset6OperandClass : AsmOperandClass {
  let Name = "HOffset6";
}
def HOffset6 : Operand<i32>, ImmLeaf<i32, [{ return isShiftedInt<6,1>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = HOffset6OperandClass;
  let PrintMethod = "printShiftedSignedImmOperand<6,1>";
  let EncoderMethod = "getShiftedSignedImmOpValue<6,1>";
  let DecoderMethod = "DecodeShiftedSignedImm<6,1>";
}

def WOffset6OperandClass : AsmOperandClass {
  let Name = "WOffset6";
}
def WOffset6 : Operand<i32>, ImmLeaf<i32, [{ return isShiftedInt<6,2>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = WOffset6OperandClass;
  let PrintMethod = "printShiftedSignedImmOperand<6,2>";
  let EncoderMethod = "getShiftedSignedImmOpValue<6,2>";
  let DecoderMethod = "DecodeShiftedSignedImm<6,2>";
}

//---
// Used by: TRAP

def TrapVect8OperandClass : AsmOperandClass {
  let Name = "TrapVect8";
}

def TrapVect8 : Operand<i8> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = TrapVect8OperandClass;
}

//---
// Used by: P_LOADCONSTH, P_LOADCONSTW

def Const16OperandClass : AsmOperandClass {
  let Name = "Const16";
}

def Const16 : Operand<i32>, ImmLeaf<i32, [{ return isInt<16>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = Const16OperandClass;
}

def Const32OperandClass : AsmOperandClass {
  let Name = "Const32";
}

def Const32 : Operand<i32>, ImmLeaf<i32, [{ return isInt<32>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = Const32OperandClass;
}
