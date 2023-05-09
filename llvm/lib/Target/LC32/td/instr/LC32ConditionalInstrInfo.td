//===- LC32ConditionalInstrInfo.td - BR Instructions -------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file deals with conditional branches - i.e. it is entirely the BR
// instruction. Almost none of this can be lowered automatically. It has custom
// code in LC32ISelLowering to select it.
//
// Also note that NOP can't be expressed with InstAlias, so we need custom code
// to deal with that.
//
//===----------------------------------------------------------------------===//

//---
// The branch itself. For conditional branches, this is emitted as part of
// C_BR_CMP_ZERO. It's also used as-is for unconditional branches.

let
  Defs = [AT],
  OutOperandList = (outs),
  isBranch = true,
  isTerminator = true
in {

  def BR : LC32Instruction {
    let AsmString = "BR$nzp\t$target";
    let InOperandList = (ins NZP:$nzp, BRTarget:$target);

    bits<3> nzp;
    bits<9> target;
    let Inst{15-12} = 0b0000;
    let Inst{11-9}  = nzp;
    let Inst{8-0}   = target;
  }

  def P_FARBR : LC32Instruction_P {
    let AsmString = "PSEUDO.FARBR$nzp\t$target";
    let InOperandList = (ins NZP:$nzp, Const32:$target);
  }
}

def : Pattern<(br bb:$target), [(BR 0b111, BRTarget:$target)]>;

//---
// Pseudo instruction to branch based on how the input compares with zero. This
// is how we encode conditional branches, and we separated it out in the
// AsmPrinter.

def NT_BR_CMP_ZERO : SDTypeProfile<
  0, 3,
  [SDTCisVT<0, i32>, SDTCisVT<1, i32>, SDTCisVT<2, OtherVT>]
>;
def N_BR_CMP_ZERO : SDNode<
  "LC32ISD::BR_CMP_ZERO",
  NT_BR_CMP_ZERO,
  [SDNPHasChain]
>;

def C_BR_CMP_ZERO : LC32Instruction_C {
  let Defs = [AT];
  let OutOperandList = (outs);
  let InOperandList = (ins NZP:$nzp, GPR:$sr, BRTarget:$target);
  let Pattern = [(N_BR_CMP_ZERO timm:$nzp, GPR:$sr, bb:$target)];

  let isBranch = true;
  let isTerminator = true;
}

//---
// Pseudo instructon for selecting based on how the input compares with zero.
// This is lowered with a custom inserter, and it desugars to branches.

def NT_SELECT_CMP_ZERO : SDTypeProfile<
  1, 4,
  [SDTCisVT<0, i32>, SDTCisVT<1, i32>, SDTCisVT<2, i32>, SDTCisVT<3, i32>, SDTCisVT<4, i32>]
>;
def N_SELECT_CMP_ZERO : SDNode<
  "LC32ISD::SELECT_CMP_ZERO",
  NT_SELECT_CMP_ZERO,
  [SDNPHasChain]
>;

def C_SELECT_CMP_ZERO : LC32Instruction_C {
  let Defs = [AT];
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins NZP:$nzp, GPR:$sr, GPR:$truev, GPR:$falsev);
  let Pattern = [(set GPR:$dr, (N_SELECT_CMP_ZERO timm:$nzp, GPR:$sr, GPR:$truev, GPR:$falsev))];

  let isSelect = true;
  let usesCustomInserter = true;
}
