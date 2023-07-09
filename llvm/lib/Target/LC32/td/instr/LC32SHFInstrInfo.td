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
// Note that shift instructions completely bypass the normal path for DAG
// instruction selection. We always want to use repeated shifts to materialize
// shifts by "large" constants, so we handle that manually. There doesn't seem
// to be a way to specify that via TableGen, so we use C++.
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

//---

// If we have an immediate mask that clears out the top or bottom bits, we want
// to realise that with shifts. Make sure to set the complexity lower than that
// of a normal ANDi so that we select it first if we can. Benchmarking, it seems
// we can use two instructions and be profitable.
//
// This adds a lot of patterns to the instruction selector. If it starts being
// slow, we should consider using raw C++ or ditching this entirely.
let AddedComplexity = -2 in {

  // Top bits
  foreach i = [1, 2, 3, 4, 5, 6, 7, 8] in {
    defvar mask = !sub(!shl(1, !sub(32, i)), 1);
    def : Pat<(and GPR:$sr1, mask), (RSHFLi (LSHFi GPR:$sr1, i), i)>;
  }

  // Bottom bits. Make sure not to generate patterns that interfere with ANDi.
  // It doesn't matter because of AddedComplexity, but we don't want to create
  // patterns that don't do anything.
  foreach i = [5, 6, 7, 8] in {
    defvar mask = !xor(!sub(!shl(1, i), 1), -1);
    def : Pat<(and GPR:$sr1, mask), (LSHFi (RSHFLi GPR:$sr1, i), i)>;
  }
}
