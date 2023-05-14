//===- LC32SHFInstrInfo.td - Shift Instructions ------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Shift instructions. That's only the register variant
// for everything except RSHFA.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

class SHFr<string name, bit a, bit d, SDNode node> : LC32Instruction {
  let AsmString = !strconcat(name, "\t$dr, $sr1, $sr2");
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$sr1, GPR:$sr2);
  let Pattern = [(set GPR:$dr, (node GPR:$sr1, GPR:$sr2))];

  bits<3> dr;
  bits<3> sr1;
  bits<3> sr2;
  let Inst{15-12} = 0b1101;
  let Inst{11-9}  = dr;
  let Inst{8-6}   = sr1;
  let Inst{5}     = 0b0;
  let Inst{4}     = a;
  let Inst{3}     = d;
  let Inst{2-0}   = sr2;
}

def LSHFr  : SHFr<"LSHF",  0b0, 0b0, shl>;
def RSHFLr : SHFr<"RSHFL", 0b0, 0b1, srl>;
def RSHFAr : SHFr<"RSHFA", 0b1, 0b1, sra>;

//---

def RSHFAi : LC32Instruction {
  let AsmString = "RHSFA\t$dr, $sr1, $amount5";
  let OutOperandList = (outs GPR:$dr);
  let InOperandList = (ins GPR:$sr1, Amount5:$amount5);
  let Pattern = [(set GPR:$dr, (sra GPR:$sr1, Amount5:$amount5))];

  bits<3> dr;
  bits<3> sr1;
  bits<5> amount5;
  let Inst{15-12} = 0b1101;
  let Inst{11-9}  = dr;
  let Inst{8-6}   = sr1;
  let Inst{5}     = 0b1;
  let Inst{4-0}   = amount5;
}

//---
// For left-shift by one, we can do that with an ADDs. It is theoretically
// possible to do this for any amount, but it might not be profitable.

def : Pat<(shl GPR:$sr, 1), (ADDr GPR:$sr, GPR:$sr)>;
