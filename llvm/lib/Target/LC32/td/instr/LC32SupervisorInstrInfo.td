//===- LC32SupervisorInstrInfo.td - Supervisor Instructions *- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Supervisor instructions: TRAP and RTI. Note that TRAP
// can change the condition codes via the service routine.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

def TRAP : Instruction {
  let Namespace = "LC32";
  let AsmString = "TRAP\t$trapvect8";

  let Defs = [CC];
  let OutOperandList = (outs);
  let InOperandList = (ins TrapVect8:$trapvect8);
  let Size = 2;

  let isTrap = true;
  let mayLoad = true;
  let mayStore = true;
  let hasSideEffects = true;

  bits<8> trapvect8;

  bits<16> Inst;
  let Inst{15-12} = 0b1111;
  let Inst{11-8}  = 0b0000;
  let Inst{7-0}   = trapvect8;
}

def GETC : InstAlias<"GETC", (TRAP 0x20)>;
def OUT  : InstAlias<"OUT",  (TRAP 0x21)>;
def PUTS : InstAlias<"PUTS", (TRAP 0x22)>;
def IN   : InstAlias<"IN",   (TRAP 0x23)>;
def HALT : InstAlias<"HALT", (TRAP 0x25)>;

def RTI : Instruction {
  let Namespace = "LC32";
  let AsmString = "RTI";

  let OutOperandList = (outs);
  let InOperandList = (ins);
  let Size = 2;

  let isReturn = true;
  let isBarrier = true;
  let mayLoad = true;
  let mayStore = false;
  let hasSideEffects = true;

  bits<16> Inst;
  let Inst{15-12} = 0b1000;
  let Inst{11-0}  = 0x000;
}
