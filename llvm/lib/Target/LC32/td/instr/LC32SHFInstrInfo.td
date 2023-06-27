//===- LC32SHFInstrInfo.td - Shift Instructions ------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the Shift instructions. That's both the register and the
// immediate variants.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

multiclass SHF<string name, bit a, bit d, SDNode node> {

  let OutOperandList = (outs GPR:$dr) in {

    def r : LC32Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $sr2");
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

    def i : LC32Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $amount3");
      let InOperandList = (ins GPR:$sr1, Amount3:$amount3);
      let Pattern = [(set GPR:$dr, (node GPR:$sr1, Amount3:$amount3))];

      bits<3> dr;
      bits<3> sr1;
      bits<3> amount3;
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
