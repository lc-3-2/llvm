//===- LC32SHFInstrInfo.td - Shift Instructions ------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Shift instructions, both the register and immediate
// variants. The format of these instructions is:
//
//                      +------+----+-----+---+---+---+-----+
//   Register Variant:  | 1101 | DR | SR1 | 0 | A | D | SR2 |
//                      +------+----+-----+---+---+---+-----+
//
//                      +------+----+----+---+---+---+---------+
//   Immediate Variant: | 1101 | DR | SR | 1 | A | D | Amount3 |
//                      +------+----+----+---+---+---+---------+
//
// Here, Amount3 is an unsigned number.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

multiclass SHF<string name, bit a, bit d, SDNode node> {

  let
    Namespace = "LC32",

    OutOperandList = (outs GPR:$dr),
    Size = 2,

    mayLoad = false,
    mayStore = false,
    hasSideEffects = false
  in {

    def r : Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $sr2");
      let InOperandList = (ins GPR:$sr1, GPR:$sr2);
      let Pattern = [(set GPR:$dr, (node GPR:$sr1, GPR:$sr2))];

      bits<3> dr;
      bits<3> sr1;
      bits<3> sr2;

      bits<16> Inst;
      let Inst{15-12} = 0b1101;
      let Inst{11-9}  = dr;
      let Inst{8-6}   = sr1;
      let Inst{5}     = 0b0;
      let Inst{4}     = a;
      let Inst{3}     = d;
      let Inst{2-0}   = sr2;
    }

    def i : Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $amount3");
      let InOperandList = (ins GPR:$sr1, Amount3:$amount3);
      let Pattern = [(set GPR:$dr, (node GPR:$sr1, Amount3:$amount3))];

      bits<3> dr;
      bits<3> sr1;
      bits<3> amount3;

      bits<16> Inst;
      let Inst{15-12} = 0b1101;
      let Inst{11-9}  = dr;
      let Inst{8-6}   = sr1;
      let Inst{5}     = 0b1;
      let Inst{4}     = a;
      let Inst{3}     = d;
      let Inst{2-0}   = amount3;
    }
  }
}

defm LSHF  : SHF<"LSHF",  0b0, 0b0, shl>;
defm RSHFL : SHF<"RSHFL", 0b0, 0b1, srl>;
defm RSHFA : SHF<"RSHFA", 0b1, 0b1, sra>;
