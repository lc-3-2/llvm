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

def : Pattern<(sextloadi8 GPR:$baser),  [(LDB GPR:$baser, 0)]>;
def : Pattern<(sextloadi16 GPR:$baser), [(LDH GPR:$baser, 0)]>;
def : Pattern<(load GPR:$baser), [(LDW GPR:$baser, 0)]>;

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

def : Pattern<(truncstorei8 GPR:$sr, GPR:$baser),  [(STB GPR:$sr, GPR:$baser, 0)]>;
def : Pattern<(truncstorei16 GPR:$sr, GPR:$baser), [(STB GPR:$sr, GPR:$baser, 0)]>;
def : Pattern<(store GPR:$sr, GPR:$baser), [(STW GPR:$sr, GPR:$baser, 0)]>;

//---

// Custom function to turn frame indicies into target frame indicies
def SelectFrameIndex : ComplexPattern<i32, 1, "SelectFrameIndex", [frameindex]>;

// Loads and stores with frame indices. We can optimize these, so we select
// here. The frame index is eliminated later.
def : Pattern<
  (load SelectFrameIndex:$fi),
  [(LDW SelectFrameIndex:$fi, 0)]
>;
def : Pattern<
  (load (add SelectFrameIndex:$fi, WOffset6:$woffset6)),
  [(LDW SelectFrameIndex:$fi, WOffset6:$woffset6)]
>;
def : Pattern<
  (store GPR:$sr, SelectFrameIndex:$fi),
  [(STW GPR:$sr, SelectFrameIndex:$fi, 0)]
>;
def : Pattern<
  (store GPR:$sr, (add SelectFrameIndex:$fi, WOffset6:$woffset6)),
  [(STW GPR:$sr, SelectFrameIndex:$fi, WOffset6:$woffset6)]
>;
