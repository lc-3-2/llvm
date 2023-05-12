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

def JMP : LC32Instruction {
  let AsmString = "JMP\t$baser";

  let OutOperandList = (outs);
  let InOperandList = (ins GPR:$baser);
  let Pattern = [(brind GPR:$baser)];

  let isBranch = true;
  let isIndirectBranch = true;
  let isBarrier = true;
  let isTerminator = true;

  bits<3> baser;
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

// Pseudo-instruction for returns. This needs to be done since otherwise the
// compiler doesn't know where returns happen.
def C_RET : LC32Instruction_C, PseudoInstExpansion<(JMP LR)> {
  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Pattern = [(N_RET)];

  let isReturn = true;
  let isBarrier = true;
  let isTerminator = true;

  let Size = 2;
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
  Uses = [SP],
  Defs = [SP],
  OutOperandList = (outs),
  InOperandList = (ins i32imm:$amtin, i32imm:$amtout),
  hasSideEffects = true
in {

  def C_ADJCALLSTACKUP : LC32Instruction_C {
    let Pattern = [(N_LC32_CALLSEQ_START timm:$amtin, timm:$amtout)];
  }

  def C_ADJCALLSTACKDOWN : LC32Instruction_C {
    let Pattern = [(N_LC32_CALLSEQ_END timm:$amtin, timm:$amtout)];
  }
}

//---

// Nodes for abstract calls. These are lowered to JSR(R) during instruction
// selection. Since all parameters are in memory, this node only takes one
// parameter - the address to call.
def NT_LC32_CALL : SDTypeProfile<0, 1, [SDTCisVT<0, iPTR>]>;
def N_LC32_CALL : SDNode<"LC32ISD::CALL", NT_LC32_CALL, [SDNPHasChain, SDNPOutGlue, SDNPOptInGlue]>;

let
  OutOperandList = (outs),
  isCall = true
in {

  def JSRR : LC32Instruction {
    let AsmString = "JSRR\t$baser";

    let Defs = [SP, LR];
    let InOperandList = (ins GPR:$baser);
    let Pattern = [(N_LC32_CALL GPR:$baser)];

    bits<3> baser;
    let Inst{15-12} = 0b0100;
    let Inst{11}    = 0b0;
    let Inst{10-9}  = 0b00;
    let Inst{8-6}   = baser;
    let Inst{5-0}   = 0b000000;
  }

  let Defs = [AT, SP, LR] in {

    def JSR : LC32Instruction {
      let AsmString = "JSR\t$pcoffset11";

      let InOperandList = (ins PCOffset11:$pcoffset11);
      let Pattern = [(N_LC32_CALL (N_ADDR_WRAPPER tglobaladdr:$pcoffset11))];

      bits<11> pcoffset11;
      let Inst{15-12} = 0b0100;
      let Inst{11}    = 0b1;
      let Inst{10-0}  = pcoffset11;
    }

    def P_FARJSR : LC32Instruction_P {
      let AsmString = "PSEUDO.FARJSR\t$addr";
      let InOperandList = (ins Const32:$addr);

      let Size = 14;
    }
  }
}

def : Pat<(N_LC32_CALL (N_ADDR_WRAPPER texternalsym:$addr)), (JSR PCOffset11:$addr)>;
