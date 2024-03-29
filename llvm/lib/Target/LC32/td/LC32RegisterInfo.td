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
// assembler needs to know about them.
//
// Notice that the condition codes are not modeled. Usually, they would be used
// to track data dependencies via implicit defs, but that's actually
// insufficient for us. Instead, we wrap condition code checking into a pseudo
// instruction. Thus, we have no need for the condition codes.
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

// If these register names change, make sure to update the code in
// * MCTargetDesc/LC32MCAsmInfo.cpp
// * LC32ISelLoweringInlineAsm.cpp
// Also make sure to update the registers in clang.
def R0 : LC32Register<"R0", 0, ["AR"]>;
def R1 : LC32Register<"R1", 1, ["XR"]>;
def R2 : LC32Register<"R2", 2, ["YR"]>;
def AT : LC32Register<"R3", 3, ["AT"]>;
def GP : LC32Register<"R4", 4, ["GP"]>;
def FP : LC32Register<"R5", 5, ["FP"]>;
def SP : LC32Register<"R6", 6, ["SP"]>;
def LR : LC32Register<"R7", 7, ["LR"]>;

//---
// Create a class of all the registers. These are in allocation order, though
// not all of them are allocateable.

def GPR : RegisterClass<"LC32", [i32], 32, (add R0, R1, R2, LR, GP, AT, FP, SP)>;
