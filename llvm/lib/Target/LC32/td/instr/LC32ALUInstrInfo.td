//===- LC32ALUInstrInfo.td - ALU Instructions --------------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file declares the ALU instructions: ADD, AND, and XOR. It also declares
// NOT as an alias for XOR.
//
// Usually: LC32InstrFormats.td and LC32InstrInfo.td
//
//===----------------------------------------------------------------------===//

multiclass ALU<string name, bits<4> opcode, SDNode node> {

  let
    Namespace = "LC32",

    Defs = [CC],
    OutOperandList = (outs GPR:$dr),

    Size = 2,

    mayLoad = false,
    mayStore = false,
    hasSideEffects = false
  in {

    def r : Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $sr2");
      let InOperandList  = (ins GPR:$sr1, GPR:$sr2);
      let Pattern = [(set GPR:$dr, (node GPR:$sr1, GPR:$sr2))];

      bits<3> dr;
      bits<3> sr1;
      bits<3> sr2;

      bits<16> Inst;
      let Inst{15-12} = opcode;
      let Inst{11-9}  = dr;
      let Inst{8-6}   = sr1;
      let Inst{5}     = 0b0;
      let Inst{4-3}   = 0b00;
      let Inst{2-0}   = sr2;
    }

    def i : Instruction {
      let AsmString = !strconcat(name, "\t$dr, $sr1, $imm5");
      let InOperandList = (ins GPR:$sr1, Imm5:$imm5);
      let Pattern = [(set GPR:$dr, (node GPR:$sr1, Imm5:$imm5))];

      bits<3> dr;
      bits<3> sr1;
      bits<5> imm5;

      bits<16> Inst;
      let Inst{15-12} = opcode;
      let Inst{11-9}  = dr;
      let Inst{8-6}   = sr1;
      let Inst{5}     = 0b1;
      let Inst{4-0}   = imm5;
    }
  }
}

defm ADD : ALU<"ADD", 0b0001, add>;
defm AND : ALU<"AND", 0b0101, and>;
defm XOR : ALU<"XOR", 0b1001, xor>;

// Don't need a pattern for NOT since LLVM already uses XOR with -1
def NOT : InstAlias<"NOT\t$dr, $sr", (XORi GPR:$dr, GPR:$sr, -1)>;
