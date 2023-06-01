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
// Used by: RSHFA

def Amount5OperandClass : AsmOperandClass {
  let Name = "Amount5";
}

def Amount5 : Operand<i32>, ImmLeaf<i32, [{ return isUInt<5>(Imm); }]> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = Amount5OperandClass;
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
// Used by: LEA, BR, JSR
//
// Note that PCOffset9 has two variants. The one used by BR is shifted left by
// one, while the one used by LEA isn't. Also, the BR variant is OtherVT since
// that is the type for basic blocks.

def PCOffset9BROperandClass : AsmOperandClass {
  let Name = "PCOffset9BR";
}

def PCOffset9BR : Operand<OtherVT> {
  let ParserMatchClass = PCOffset9BROperandClass;
  let PrintMethod = "printPCOffset<9,1>";
  let EncoderMethod = "getPCOffsetValue<9,1>";
  let DecoderMethod = "DecodePCOffset<9,1>";
}

def PCOffset9LEAOperandClass : AsmOperandClass {
  let Name = "PCOffset9LEA";
}

def PCOffset9LEA : Operand<iPTR> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = PCOffset9LEAOperandClass;
  let PrintMethod = "printPCOffset<9,0>";
  let EncoderMethod = "getPCOffsetValue<9,0>";
  let DecoderMethod = "DecodePCOffset<9,0>";
}

def PCOffset11OperandClass : AsmOperandClass {
  let Name = "PCOffset11";
}

def PCOffset11 : Operand<iPTR> {
  let OperandType = "OPERAND_IMMEDIATE";
  let ParserMatchClass = PCOffset11OperandClass;
  let PrintMethod = "printPCOffset<11,1>";
  let EncoderMethod = "getPCOffsetValue<11,1>";
  let DecoderMethod = "DecodePCOffset<11,1>";
}

//---
// Used by: BR
//
// Internally, the NZP fields are are Imm, encoded with bits. For instance, `n`
// would be encoded as `0b100`. They have to be i32 since that's what's legal.

def NZPOperandClass : AsmOperandClass {
  let Name = "NZP";
}

def NZP : Operand<i32> {
  let ParserMatchClass = NZPOperandClass;
  let PrintMethod = "printNZP";
}

//---
// Used by: TRAP

def TrapVect8OperandClass : AsmOperandClass {
  let Name = "TrapVect8";
}

def TrapVect8 : Operand<i32> {
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
