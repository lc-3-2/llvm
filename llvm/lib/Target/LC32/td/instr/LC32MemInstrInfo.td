//===- LC32MemInstrInfo.td - Memory Instructions -----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains declarations for all the memory operations, so LD* and
// ST*. It also contains pattern matching code for them.
//
// TODO: Find a good way to macro the patterns. As is, this copy-pasta
// programming is error-prone.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def LDB : Instruction {
  let Namespace = "LC32";
  let AsmString = "LDB\t$dr, $baser, $boffset6";

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$baser, BOffset6:$boffset6);
  let Size = 2;

  let Pattern =
    [(set GPR:$dr, (sextloadi8 (add GPR:$baser, BOffset6:$boffset6)))];

  let mayLoad = true;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> dr;
  bits<3> baser;
  bits<6> boffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b0010;
  let Inst{11-9}  = dr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = boffset6;
}

def : Pattern<(extloadi8 (add GPR:$baser, BOffset6:$boffset6)), [(LDB GPR:$baser, BOffset6:$boffset6)]>;
def : Pattern<(sextloadi8 GPR:$baser), [(LDB GPR:$baser, 0)]>;
def : Pattern<(extloadi8 GPR:$baser),  [(LDB GPR:$baser, 0)]>;

def LDH : Instruction {
  let Namespace = "LC32";
  let AsmString = "LDH\t$dr, $baser, $hoffset6";

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$baser, HOffset6:$hoffset6);
  let Size = 2;

  let Pattern =
    [(set GPR:$dr, (sextloadi16 (add GPR:$baser, HOffset6:$hoffset6)))];

  let mayLoad = true;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> dr;
  bits<3> baser;
  bits<6> hoffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b0110;
  let Inst{11-9}  = dr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = hoffset6;
}

def : Pattern<(extloadi16 (add GPR:$baser, HOffset6:$hoffset6)), [(LDH GPR:$baser, HOffset6:$hoffset6)]>;
def : Pattern<(sextloadi16 GPR:$baser), [(LDH GPR:$baser, 0)]>;
def : Pattern<(extloadi16 GPR:$baser),  [(LDH GPR:$baser, 0)]>;

def LDW : Instruction {
  let Namespace = "LC32";
  let AsmString = "LDW\t$dr, $baser, $woffset6";

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$baser, WOffset6:$woffset6);
  let Size = 2;

  let Pattern = [(set GPR:$dr, (load (add GPR:$baser, WOffset6:$woffset6)))];

  let mayLoad = true;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> dr;
  bits<3> baser;
  bits<6> woffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b1010;
  let Inst{11-9}  = dr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = woffset6;
}

def : Pattern<(sextloadi32 (add GPR:$baser, WOffset6:$woffset6)), [(LDW GPR:$baser, WOffset6:$woffset6)]>;
def : Pattern<(extloadi32 (add GPR:$baser, WOffset6:$woffset6)),  [(LDW GPR:$baser, WOffset6:$woffset6)]>;
def : Pattern<(sextloadi32 GPR:$baser), [(LDW GPR:$baser, 0)]>;
def : Pattern<(extloadi32 GPR:$baser),  [(LDW GPR:$baser, 0)]>;
def : Pattern<(load GPR:$baser),        [(LDW GPR:$baser, 0)]>;

//---

def STB : Instruction {
  let Namespace = "LC32";
  let AsmString = "STB\t$sr, $baser, $boffset6";

  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$sr, GPR:$baser, BOffset6:$boffset6);
  let Size = 2;

  let Pattern = [(truncstorei8 GPR:$sr, (add GPR:$baser, BOffset6:$boffset6))];

  let mayLoad = false;
  let mayStore = true;
  let hasSideEffects = false;

  bits<3> sr;
  bits<3> baser;
  bits<6> boffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b0011;
  let Inst{11-9}  = sr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = boffset6;
}

def : Pattern<(truncstorei8 GPR:$sr, GPR:$baser),  [(STB GPR:$sr, GPR:$baser, 0)]>;

def STH : Instruction {
  let Namespace = "LC32";
  let AsmString = "STH\t$sr, $baser, $hoffset6";

  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$sr, GPR:$baser, HOffset6:$hoffset6);
  let Size = 2;

  let Pattern = [(truncstorei16 GPR:$sr, (add GPR:$baser, HOffset6:$hoffset6))];

  let mayLoad = false;
  let mayStore = true;
  let hasSideEffects = false;

  bits<3> sr;
  bits<3> baser;
  bits<6> hoffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b0111;
  let Inst{11-9}  = sr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = hoffset6;
}

def : Pattern<(truncstorei16 GPR:$sr, GPR:$baser), [(STH GPR:$sr, GPR:$baser, 0)]>;

def STW : Instruction {
  let Namespace = "LC32";
  let AsmString = "STW\t$sr, $baser, $woffset6";

  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$sr, GPR:$baser, WOffset6:$woffset6);
  let Size = 2;

  let Pattern = [(store GPR:$sr, (add GPR:$baser, WOffset6:$woffset6))];

  let mayLoad = false;
  let mayStore = true;
  let hasSideEffects = false;

  bits<3> sr;
  bits<3> baser;
  bits<6> woffset6;

  bits<16> Inst;
  let Inst{15-12} = 0b1011;
  let Inst{11-9}  = sr;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = woffset6;
}

def : Pattern<(truncstorei32 GPR:$sr, (add GPR:$baser, WOffset6:$woffset6)), [(STW GPR:$sr, GPR:$baser, WOffset6:$woffset6)]>;
def : Pattern<(truncstorei32 GPR:$sr, GPR:$baser), [(STW GPR:$sr, GPR:$baser, 0)]>;
def : Pattern<(store GPR:$sr, GPR:$baser),         [(STW GPR:$sr, GPR:$baser, 0)]>;

//---

// Custom function to turn frame indicies into target frame indicies
def SelectFrameIndex : ComplexPattern<i32, 1, "SelectFrameIndex", [frameindex]>;

// Loads and stores with frame indices. We can optimize these, so we select
// here. The frame index is eliminated later.

def : Pattern<(load SelectFrameIndex:$fi), [(LDW SelectFrameIndex:$fi, 0)]>;
def : Pattern<(load (add SelectFrameIndex:$fi, WOffset6:$woffset6)), [(LDW SelectFrameIndex:$fi, WOffset6:$woffset6)]>;

def : Pattern<(store GPR:$sr, SelectFrameIndex:$fi), [(STW GPR:$sr, SelectFrameIndex:$fi, 0)]>;
def : Pattern<(store GPR:$sr, (add SelectFrameIndex:$fi, WOffset6:$woffset6)), [(STW GPR:$sr, SelectFrameIndex:$fi, WOffset6:$woffset6)]>;

def : Pattern<(extloadi8 SelectFrameIndex:$fi),   [(LDB SelectFrameIndex:$fi, 0)]>;
def : Pattern<(sextloadi8 SelectFrameIndex:$fi),  [(LDB SelectFrameIndex:$fi, 0)]>;
def : Pattern<(extloadi16 SelectFrameIndex:$fi),  [(LDH SelectFrameIndex:$fi, 0)]>;
def : Pattern<(sextloadi16 SelectFrameIndex:$fi), [(LDH SelectFrameIndex:$fi, 0)]>;
def : Pattern<(extloadi32 SelectFrameIndex:$fi),  [(LDW SelectFrameIndex:$fi, 0)]>;
def : Pattern<(sextloadi32 SelectFrameIndex:$fi), [(LDW SelectFrameIndex:$fi, 0)]>;
def : Pattern<(extloadi8 (add SelectFrameIndex:$fi, BOffset6:$boffset6)),   [(LDB SelectFrameIndex:$fi, BOffset6:$boffset6)]>;
def : Pattern<(sextloadi8 (add SelectFrameIndex:$fi, BOffset6:$boffset6)),  [(LDB SelectFrameIndex:$fi, BOffset6:$boffset6)]>;
def : Pattern<(extloadi16 (add SelectFrameIndex:$fi, HOffset6:$hoffset6)),  [(LDH SelectFrameIndex:$fi, HOffset6:$hoffset6)]>;
def : Pattern<(sextloadi16 (add SelectFrameIndex:$fi, HOffset6:$hoffset6)), [(LDH SelectFrameIndex:$fi, HOffset6:$hoffset6)]>;
def : Pattern<(extloadi32 (add SelectFrameIndex:$fi, WOffset6:$woffset6)),  [(LDH SelectFrameIndex:$fi, WOffset6:$woffset6)]>;
def : Pattern<(sextloadi32 (add SelectFrameIndex:$fi, WOffset6:$woffset6)), [(LDH SelectFrameIndex:$fi, WOffset6:$woffset6)]>;

def : Pattern<(truncstorei8 GPR:$sr, SelectFrameIndex:$fi),  [(STB GPR:$sr, SelectFrameIndex:$fi, 0)]>;
def : Pattern<(truncstorei16 GPR:$sr, SelectFrameIndex:$fi), [(STH GPR:$sr, SelectFrameIndex:$fi, 0)]>;
def : Pattern<(truncstorei32 GPR:$sr, SelectFrameIndex:$fi), [(STW GPR:$sr, SelectFrameIndex:$fi, 0)]>;
def : Pattern<(truncstorei8 GPR:$sr, (add SelectFrameIndex:$fi, BOffset6:$boffset6)),  [(STB GPR:$sr, SelectFrameIndex:$fi, BOffset6:$boffset6)]>;
def : Pattern<(truncstorei16 GPR:$sr, (add SelectFrameIndex:$fi, HOffset6:$hoffset6)), [(STH GPR:$sr, SelectFrameIndex:$fi, HOffset6:$hoffset6)]>;
def : Pattern<(truncstorei32 GPR:$sr, (add SelectFrameIndex:$fi, WOffset6:$woffset6)), [(STH GPR:$sr, SelectFrameIndex:$fi, WOffset6:$woffset6)]>;

def C_LEA_FRAMEINDEX : Instruction {
  // For when we can't eliminate the frame index via a load. This computes the
  // address we store to. This is lowered when we eliminate frame indices,
  // instead of in the AsmPrinter like most of the other pseudo instructions.

  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let Defs = [CC];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins i32imm:$fi);

  let Pattern = [(set GPR:$dr, SelectFrameIndex:$fi)];

  let isReMaterializable = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

//---

// Custom functions to turn addresses into their target variant
def SelectGlobalAddress  : ComplexPattern<i32, 1, "SelectGlobalAddress",  [globaladdr]>;
def SelectExternalSymbol : ComplexPattern<i32, 1, "SelectExternalSymbol", [externalsym]>;
def SelectConstantPool   : ComplexPattern<i32, 1, "SelectConstantPool",   [constpool]>;
def SelectJumpTable      : ComplexPattern<i32, 1, "SelectJumpTable",      [jumptable]>;
def SelectBlockAddress   : ComplexPattern<i32, 1, "SelectBlockAddress",   [blockaddress]>;

def LEA : Instruction {
  let Namespace = "LC32";
  let AsmString = "LEA\t$dr, $pcoffset9";

  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins PCOffset9:$pcoffset9);
  let Size = 2;

  let Pattern = [(set GPR:$dr, SelectGlobalAddress:$pcoffset9)];

  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> dr;
  bits<9> pcoffset9;

  bits<16> Inst;
  let Inst{15-12} = 0b1110;
  let Inst{11-9}  = dr;
  let Inst{8-0}   = pcoffset9;
}

def : Pattern<(SelectExternalSymbol:$addr), [(LEA SelectExternalSymbol:$addr)]>;
def : Pattern<(SelectConstantPool:$addr),   [(LEA SelectConstantPool:$addr)]>;
def : Pattern<(SelectJumpTable:$addr),      [(LEA SelectJumpTable:$addr)]>;
def : Pattern<(SelectBlockAddress:$addr),   [(LEA SelectBlockAddress:$addr)]>;
