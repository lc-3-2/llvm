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

// If we have an immediate mask that clears out the top bits, we want to realise
// that with shifts. For profitability, we have a budget of four instructions.
// Set the complexity to be lower than the normal immediate AND so that its
// selected first if possible.
//
// Note that this could make instruction selection much slower because of how
// many patterns we're generating. Hopefully it won't be an issue, but if it is
// we should select these with C++ or ditch them entirely.
foreach i = [1, 2, 3, 4, 5, 6, 7, 8] in {
  defvar mask0 = !sub(!shl(1, !sub(!sub(32, 0), i)), 1);
  defvar mask1 = !sub(!shl(1, !sub(!sub(32, 8), i)), 1);

  let AddedComplexity = -2 in {
    def : Pat<(and GPR:$sr1, mask0), (RSHFLi (LSHFi GPR:$sr1, i), i)>;
    def : Pat<(and GPR:$sr1, mask1), (RSHFLi (RSHFLi (LSHFi (LSHFi GPR:$sr1, 8), i), 8), i)>;
  }
}

// Do the same as above for clearing the low bits. Again, this could make
// instruction selection much slower.
foreach i = [1, 2, 3, 4, 5, 6, 7, 8] in {
  defvar mask0 = !xor(!sub(!shl(1, !add(i, 0)), 1), -1);
  defvar mask1 = !xor(!sub(!shl(1, !add(i, 8)), 1), -1);

  // We could use both RSHFA and RSHFL here. I pick RSHFA because it
  // distinguishes from the other case where we have to use RSHFL.
  let AddedComplexity = -2 in {
    // Extra check for mask0 to make sure it doesn't interfere with the normal
    // immediate AND. We can get by without it due to AddedComplexity, but we
    // want to keep the table size small, so we should keep unneeeded patterns
    // to a minimum.
    if !gt(i, 4) then {
      def : Pat<(and GPR:$sr1, mask0), (LSHFi (RSHFAi GPR:$sr1, i), i)>;
    }
    // Usual mask1 code
    def : Pat<(and GPR:$sr1, mask1), (LSHFi (LSHFi (RSHFAi (RSHFAi GPR:$sr1, 8), i), 8), i)>;
  }
}
