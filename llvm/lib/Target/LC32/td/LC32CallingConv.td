//===- LC32CallingConv.td - Calling Convention for LC-3.2 ---*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file details the calling convention for the LC-3.2. It assigns
// everything to the stack for calls, converting to i32 if possible. For
// returns, it tries to return an i32, otherwise it does sret. For by value
// struct passing, we align it to at least an i32 because convention. For the
// return values, we only do the first one.
//
// Note that the promotions, while present, are not needed. LLVM will
// automatically make the type a register that can fit it. This means promotion
// happens without intervention.
//
// Usually: LC32CallingConv.td
// See: LC32ISelLowering.cpp
//
//===----------------------------------------------------------------------===//

// Arguments
def LC32CallingConv : CallingConv<[
  CCIfByVal<CCPassByVal<4, 4>>,
  CCIfType<[i1, i4, i8, i16], CCPromoteToType<i32>>,
  CCAssignToStack<0, 0>
]>;

// Return values
def LC32RetCallingConv : CallingConv<[
  CCIfType<[i1, i4, i8, i16], CCPromoteToType<i32>>,
  CCIfType<[i32], CCIf<"ValNo == 0", CCAssignToStack<4, 4>>>,
]>;
