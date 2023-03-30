//===- LC32RegisterInfo.td - Register Definitions ----------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file specifies the registers for the LC-3.2. It specifies all the
// general-purpose registers, even ones that can't be allocated since the
// assembler needs to know about them. It also defines a register for the
// condition codes to track the data dependence.
//
// Usually: LC32RegisterInfo.td
//
//===----------------------------------------------------------------------===//

// ---
// Model general-purpose registers. These are R0-R7, but in the code we give
// special names to R4-R7 to remind of their special purpose. Only R0-R2 are
// availble to the register allocator.

class LC32Register<
  string name,
  bits<3> num,
  list<string> altNames = []
> : Register<name, altNames> {
  let Namespace = "LC32";
  let HWEncoding{2-0} = num;
  let DwarfNumbers = [num];
}

def R0 : LC32Register<"R0", 0>;
def R1 : LC32Register<"R1", 1>;
def R2 : LC32Register<"R2", 2>;
def AT : LC32Register<"R3", 3, ["AT"]>;
def GP : LC32Register<"R4", 4, ["GP"]>;
def FP : LC32Register<"R5", 5, ["FP"]>;
def SP : LC32Register<"R6", 6, ["SP"]>;
def LR : LC32Register<"R7", 7, ["LR"]>;

def GPR : RegisterClass<"LC32", [i32], 32, (add R0, R1, R2, AT, GP, FP, SP, LR)>;

//---
// Model condition codes. This is a data depencence between instructions,
// particularly for branches. We need to capture this.

def CC : Register<"CC", ["PSR"]> {
  let Namespace = "LC32";
}
