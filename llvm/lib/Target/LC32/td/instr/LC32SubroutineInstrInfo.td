//===- LC32SubroutineInstrInfo.td - Subroutine Management --*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains declarations related to subroutines. It contains JSR and
// RET, but it also contains SelectionDAG nodes to select the call and return.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def JMP : Instruction {
  let Namespace = "LC32";
  let AsmString = "JMP\t$baser";

  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$baser);
  let Size = 2;

  let Pattern = [(brind GPR:$baser)];

  let isBranch = true;
  let isIndirectBranch = true;
  let isBarrier = true;
  let isTerminator = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = false;

  bits<3> baser;

  bits<16> Inst;
  let Inst{15-12} = 0b1100;
  let Inst{11-9}  = 0b000;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = 0b000000;
}

def RET : InstAlias<"RET", (JMP LR)>;

//---

// SelectionDAG node for returns
// See: C_RET
def N_RET : SDNode<"LC32ISD::RET", SDTNone, [SDNPHasChain]>;

def C_RET : Instruction, PseudoInstExpansion<(JMP LR)> {
  // Pseudo-instruction for returns. This needs to be done since otherwise the
  // compiler doesn't know where returns happen.

  let Namespace = "LC32";
  let isPseudo = true;
  let isCodeGenOnly = true;

  let OutOperandList = (outs);
  let InOperandList = (ins);

  let Pattern = [(N_RET)];

  let isReturn = true;
  let isBarrier = true;
  let isTerminator = true;
  let mayLoad = false;
  let mayStore = false;
  let hasSideEffects = true;
}
