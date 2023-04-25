//===- LC32CallingConv.td - Calling Convention for LC-3.2 ---*- tablegen -*-==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file details the calling convention for the LC-3.2. It converts
// everything to 32-bit integers, then assigns them to the stack. This goes for
// both arguments and return values.
//
// For the return values, we only do the first one. Essentially, this amounts to
// only assigning to the stack if the stack is currently empty.
//
// Usually: LC32CallingConv.td
// See: LC32ISelLowering.cpp
//
//===----------------------------------------------------------------------===//

// Arguments
def LC32CallingConv : CallingConv<[
  CCIfType<[i1, i4, i8, i16], CCPromoteToType<i32>>,
  CCAssignToStack<4, 4>
]>;

// Return values
def LC32RetCallingConv : CallingConv<[
  CCIfType<[i1, i4, i8, i16], CCPromoteToType<i32>>,
  CCIfType<[i32], CCIf<"State.getNextStackOffset() == 0", CCAssignToStack<4, 4>>>,
]>;
