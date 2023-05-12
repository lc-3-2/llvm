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

//---
// Loads

let
  OutOperandList = (outs GPR:$dr),
  mayLoad = true
in {

  def LDB : LC32Instruction {
    let AsmString = "LDB\t$dr, $baser, $boffset6";

    let InOperandList = (ins GPR:$baser, BOffset6:$boffset6);
    let Pattern = [(set GPR:$dr, (sextloadi8 (add GPR:$baser, BOffset6:$boffset6)))];

    bits<3> dr;
    bits<3> baser;
    bits<6> boffset6;
    let Inst{15-12} = 0b0010;
    let Inst{11-9}  = dr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = boffset6;
  }

  def LDH : LC32Instruction {
    let AsmString = "LDH\t$dr, $baser, $hoffset6";

    let InOperandList = (ins GPR:$baser, HOffset6:$hoffset6);
    let Pattern = [(set GPR:$dr, (sextloadi16 (add GPR:$baser, HOffset6:$hoffset6)))];

    bits<3> dr;
    bits<3> baser;
    bits<6> hoffset6;
    let Inst{15-12} = 0b0110;
    let Inst{11-9}  = dr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = hoffset6;
  }

  def LDW : LC32Instruction {
    let AsmString = "LDW\t$dr, $baser, $woffset6";

    let InOperandList = (ins GPR:$baser, WOffset6:$woffset6);
    let Pattern = [(set GPR:$dr, (load (add GPR:$baser, WOffset6:$woffset6)))];

    bits<3> dr;
    bits<3> baser;
    bits<6> woffset6;
    let Inst{15-12} = 0b1010;
    let Inst{11-9}  = dr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = woffset6;
  }
}

def : Pat<
  (extloadi8 (add GPR:$baser, BOffset6:$boffset6)),
  (LDB GPR:$baser, BOffset6:$boffset6)
>;
def : Pat<(sextloadi8 GPR:$baser), (LDB GPR:$baser, 0)>;
def : Pat<(extloadi8 GPR:$baser),  (LDB GPR:$baser, 0)>;

def : Pat<
  (extloadi16 (add GPR:$baser, HOffset6:$hoffset6)),
  (LDH GPR:$baser, HOffset6:$hoffset6)
>;
def : Pat<(sextloadi16 GPR:$baser), (LDH GPR:$baser, 0)>;
def : Pat<(extloadi16 GPR:$baser),  (LDH GPR:$baser, 0)>;

def : Pat<
  (sextloadi32 (add GPR:$baser, WOffset6:$woffset6)),
  (LDW GPR:$baser, WOffset6:$woffset6)
>;
def : Pat<
  (extloadi32 (add GPR:$baser, WOffset6:$woffset6)),
  (LDW GPR:$baser, WOffset6:$woffset6)
>;
def : Pat<(sextloadi32 GPR:$baser), (LDW GPR:$baser, 0)>;
def : Pat<(extloadi32 GPR:$baser),  (LDW GPR:$baser, 0)>;
def : Pat<(load GPR:$baser),        (LDW GPR:$baser, 0)>;

//---
// Stores

let
  OutOperandList = (outs),
  mayStore = true
in {

  def STB : LC32Instruction {
    let AsmString = "STB\t$sr, $baser, $boffset6";

    let InOperandList = (ins GPR:$sr, GPR:$baser, BOffset6:$boffset6);
    let Pattern = [(truncstorei8 GPR:$sr, (add GPR:$baser, BOffset6:$boffset6))];

    bits<3> sr;
    bits<3> baser;
    bits<6> boffset6;
    let Inst{15-12} = 0b0011;
    let Inst{11-9}  = sr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = boffset6;
  }

  def STH : LC32Instruction {
    let AsmString = "STH\t$sr, $baser, $hoffset6";

    let InOperandList = (ins GPR:$sr, GPR:$baser, HOffset6:$hoffset6);
    let Pattern = [(truncstorei16 GPR:$sr, (add GPR:$baser, HOffset6:$hoffset6))];

    bits<3> sr;
    bits<3> baser;
    bits<6> hoffset6;
    let Inst{15-12} = 0b0111;
    let Inst{11-9}  = sr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = hoffset6;
  }

  def STW : LC32Instruction {
    let AsmString = "STW\t$sr, $baser, $woffset6";

    let InOperandList = (ins GPR:$sr, GPR:$baser, WOffset6:$woffset6);
    let Pattern = [(store GPR:$sr, (add GPR:$baser, WOffset6:$woffset6))];

    bits<3> sr;
    bits<3> baser;
    bits<6> woffset6;
    let Inst{15-12} = 0b1011;
    let Inst{11-9}  = sr;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = woffset6;
  }
}

def : Pat<(truncstorei8 GPR:$sr, GPR:$baser), (STB GPR:$sr, GPR:$baser, 0)>;

def : Pat<(truncstorei16 GPR:$sr, GPR:$baser), (STH GPR:$sr, GPR:$baser, 0)>;

def : Pat<
  (truncstorei32 GPR:$sr, (add GPR:$baser, WOffset6:$woffset6)),
  (STW GPR:$sr, GPR:$baser, WOffset6:$woffset6)
>;
def : Pat<(truncstorei32 GPR:$sr, GPR:$baser), (STW GPR:$sr, GPR:$baser, 0)>;
def : Pat<(store GPR:$sr, GPR:$baser),         (STW GPR:$sr, GPR:$baser, 0)>;

//---
// Address manipulation
//
// Addresses can't be selected when they are the top-most node. If you try, the
// matcher just uses the result as the source, which is not what we want.
// Therefore, we take the approach of most backends and create a wrapper node
// around addresses. Logically they just pass the value through, but we can use
// them as the root for selection.
//
// These wrappers are constructed in ISelLowering - we do Custom expansion. We
// also add LEA to load from these wrappers, as well as an instruction to load
// the address of a frame index. The former can be relaxed to PSEUDO.LOADCONSTW,
// and the latter is lowered along with frame indicies.

def NT_ADDR_WRAPPER : SDTypeProfile<1, 1, [
  SDTCisPtrTy<0>, SDTCisPtrTy<1>, SDTCisSameAs<0, 1>
]>;
def N_ADDR_WRAPPER : SDNode<"LC32ISD::ADDR_WRAPPER", NT_ADDR_WRAPPER>;

let
  OutOperandList = (outs GPR:$dr),
  isReMaterializable = true
in {

  def C_LEA_FRAMEINDEX : LC32Instruction_C {
    let InOperandList = (ins i32imm:$fi);
    let Pattern = [(set GPR:$dr, (N_ADDR_WRAPPER tframeindex:$fi))];
  }

  def LEA : LC32Instruction {
    let AsmString = "LEA\t$dr, $pcoffset9";

    let InOperandList = (ins PCOffset9:$pcoffset9);
    let Pattern = [(set GPR:$dr, (N_ADDR_WRAPPER tglobaladdr:$pcoffset9))];

    bits<3> dr;
    bits<9> pcoffset9;
    let Inst{15-12} = 0b1110;
    let Inst{11-9}  = dr;
    let Inst{8-0}   = pcoffset9;
  }
}

def : Pat<(N_ADDR_WRAPPER texternalsym:$addr),  (LEA PCOffset9:$addr)>;
def : Pat<(N_ADDR_WRAPPER tconstpool:$addr),    (LEA PCOffset9:$addr)>;
def : Pat<(N_ADDR_WRAPPER tjumptable:$addr),    (LEA PCOffset9:$addr)>;
def : Pat<(N_ADDR_WRAPPER tblockaddress:$addr), (LEA PCOffset9:$addr)>;

//--
// Loads and stores with frame indices. We can optimize these, so we select
// here. The frame index is eliminated later.

def : Pat<(load (N_ADDR_WRAPPER tframeindex:$fi)), (LDW tframeindex:$fi, 0)>;
def : Pat<
  (load (add (N_ADDR_WRAPPER tframeindex:$fi), WOffset6:$woffset6)),
  (LDW tframeindex:$fi, WOffset6:$woffset6)
>;

def : Pat<(store GPR:$sr, (N_ADDR_WRAPPER tframeindex:$fi)), (STW GPR:$sr, tframeindex:$fi, 0)>;
def : Pat<
  (store GPR:$sr, (add (N_ADDR_WRAPPER tframeindex:$fi), WOffset6:$woffset6)),
  (STW GPR:$sr, tframeindex:$fi, WOffset6:$woffset6)
>;

def : Pat<(extloadi8 (N_ADDR_WRAPPER tframeindex:$fi)),   (LDB tframeindex:$fi, 0)>;
def : Pat<(sextloadi8 (N_ADDR_WRAPPER tframeindex:$fi)),  (LDB tframeindex:$fi, 0)>;
def : Pat<(extloadi16 (N_ADDR_WRAPPER tframeindex:$fi)),  (LDH tframeindex:$fi, 0)>;
def : Pat<(sextloadi16 (N_ADDR_WRAPPER tframeindex:$fi)), (LDH tframeindex:$fi, 0)>;
def : Pat<(extloadi32 (N_ADDR_WRAPPER tframeindex:$fi)),  (LDW tframeindex:$fi, 0)>;
def : Pat<(sextloadi32 (N_ADDR_WRAPPER tframeindex:$fi)), (LDW tframeindex:$fi, 0)>;
def : Pat<
  (extloadi8 (add (N_ADDR_WRAPPER tframeindex:$fi), BOffset6:$boffset6)),
  (LDB tframeindex:$fi, BOffset6:$boffset6)
>;
def : Pat<
  (sextloadi8 (add (N_ADDR_WRAPPER tframeindex:$fi), BOffset6:$boffset6)),
  (LDB tframeindex:$fi, BOffset6:$boffset6)
>;
def : Pat<
  (extloadi16 (add (N_ADDR_WRAPPER tframeindex:$fi), HOffset6:$hoffset6)),
  (LDH tframeindex:$fi, HOffset6:$hoffset6)
>;
def : Pat<
  (sextloadi16 (add (N_ADDR_WRAPPER tframeindex:$fi), HOffset6:$hoffset6)),
  (LDH tframeindex:$fi, HOffset6:$hoffset6)
>;
def : Pat<
  (extloadi32 (add (N_ADDR_WRAPPER tframeindex:$fi), WOffset6:$woffset6)),
  (LDW tframeindex:$fi, WOffset6:$woffset6)
>;
def : Pat<
  (sextloadi32 (add (N_ADDR_WRAPPER tframeindex:$fi), WOffset6:$woffset6)),
  (LDW tframeindex:$fi, WOffset6:$woffset6)
>;

def : Pat<(truncstorei8 GPR:$sr, (N_ADDR_WRAPPER tframeindex:$fi)),  (STB GPR:$sr, tframeindex:$fi, 0)>;
def : Pat<(truncstorei16 GPR:$sr, (N_ADDR_WRAPPER tframeindex:$fi)), (STH GPR:$sr, tframeindex:$fi, 0)>;
def : Pat<(truncstorei32 GPR:$sr, (N_ADDR_WRAPPER tframeindex:$fi)), (STW GPR:$sr, tframeindex:$fi, 0)>;
def : Pat<
  (truncstorei8 GPR:$sr, (add (N_ADDR_WRAPPER tframeindex:$fi), BOffset6:$boffset6)),
  (STB GPR:$sr, tframeindex:$fi, BOffset6:$boffset6)
>;
def : Pat<
  (truncstorei16 GPR:$sr, (add (N_ADDR_WRAPPER tframeindex:$fi), HOffset6:$hoffset6)),
  (STH GPR:$sr, tframeindex:$fi, HOffset6:$hoffset6)
>;
def : Pat<
  (truncstorei32 GPR:$sr, (add (N_ADDR_WRAPPER tframeindex:$fi), WOffset6:$woffset6)),
  (STW GPR:$sr, tframeindex:$fi, WOffset6:$woffset6)
>;
