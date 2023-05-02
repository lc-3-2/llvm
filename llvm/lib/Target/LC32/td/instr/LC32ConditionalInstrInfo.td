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
// Comparison against zero. This node outputs a glue value, which is used by the
// conditional proper.

def NT_CMP_ZERO : SDTypeProfile<0, 1, [SDTCisInt<0>]>;
def N_CMP_ZERO : SDNode<"LC32ISD::CMP_ZERO", NT_CMP_ZERO, [SDNPOutGlue]>;

def CMP_ZERO : Instruction, PseudoInstExpansion<(ADDi GPR:$sr, GPR:$sr, 0)> {
  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let Defs = [CC];
  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$sr);

  let Pattern = [(N_CMP_ZERO GPR:$sr)];

  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;
}

//---
// The branch

def NT_BR : SDTypeProfile<0, 2, [SDTCisVT<0, i8>, SDTCisVT<1, OtherVT>]>;
def N_BR : SDNode<"LC32ISD::BR", NT_BR, [SDNPHasChain, SDNPOptInGlue]>;

def BR : Instruction {
  let Namespace = "LC32";
  let AsmString = "BR$nzp\t$pcoffset9";

  let Defs = [AT, CC];
  let Uses = [CC];
  let OutOperandList = (outs);
  let InOperandList = (ins NZP:$nzp, BRTarget:$pcoffset9);
  let Size = 2;

  let Pattern = [(N_BR NZP:$nzp, bb:$pcoffset9)];

  let isBranch = true;
  let isTerminator = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> nzp;
  bits<9> pcoffset9;

  bits<16> Inst;
  let Inst{15-12} = 0b0000;
  let Inst{11-9}  = nzp;
  let Inst{8-0}   = pcoffset9;
}

def : Pattern<(br bb:$pcoffset9), [(BR 0b111, BRTarget:$pcoffset9)]>;
