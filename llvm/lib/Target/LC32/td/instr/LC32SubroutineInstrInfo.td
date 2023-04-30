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

//---

// Nodes for call sequencing. This data is shared across targets, but the
// arguments are different. Thus, we have to define it outselves
def NT_LC32_CALLSEQ_START : SDCallSeqStart<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def NT_LC32_CALLSEQ_END   : SDCallSeqEnd<[SDTCisVT<0, i32>, SDTCisVT<1, i32>]>;
def N_LC32_CALLSEQ_START : SDNode<"ISD::CALLSEQ_START", NT_LC32_CALLSEQ_START, [SDNPHasChain, SDNPOutGlue]>;
def N_LC32_CALLSEQ_END   : SDNode<"ISD::CALLSEQ_END",   NT_LC32_CALLSEQ_END,   [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue]>;

// Use pseudo instructions to match call sequencing operations. These are
// eliminated in LC32InstrInfo and LC32FrameLowering.
let
  Namespace = "LC32",
  isPseudo = true,
  isCodeGenOnly = true,

  Uses = [SP],
  Defs = [SP, CC],
  OutOperandList = (outs),
  InOperandList = (ins i32imm:$amtin, i32imm:$amtout),

  mayLoad = false,
  mayStore = false,
  hasSideEffects = true
in {

  def C_ADJCALLSTACKUP : Instruction {
    let Pattern = [(N_LC32_CALLSEQ_START timm:$amtin, timm:$amtout)];
  }

  def C_ADJCALLSTACKDOWN : Instruction {
    let Pattern = [(N_LC32_CALLSEQ_END timm:$amtin, timm:$amtout)];
  }
}

//---

// Nodes for abstract calls. These are lowered to JSR(R) during instruction
// selection. Since all parameters are in memory, this node only takes one
// parameter - the address to call.
def NT_LC32_CALL : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def N_LC32_CALL : SDNode<"LC32ISD::CALL", NT_LC32_CALL, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

def JSRR : Instruction {
  let Namespace = "LC32";
  let AsmString = "JSRR\t$baser";

  let Defs = [SP, CC];
  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$baser);
  let Size = 2;

  let Pattern = [(N_LC32_CALL GPR:$baser)];

  let isCall = true;
  let hasSideEffects = true;

  bits<3> baser;

  bits<16> Inst;
  let Inst{15-12} = 0b0100;
  let Inst{11}    = 0b0;
  let Inst{10-9}  = 0b00;
  let Inst{8-6}   = baser;
  let Inst{5-0}   = 0b000000;
}
