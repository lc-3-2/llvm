//===- LC32.td - Describe the LC-3.2 Target Machine --------*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

include "td/LC32RegisterInfo.td"
include "td/LC32InstrInfo.td"
include "td/LC32CallingConv.td"
include "td/LC32Schedule.td"

include "td/LC32AsmParser.td"

def LC32 : Target {
  // Defined in: td/LC32InstrInfo.td
  let InstructionSet = LC32InstrInfo;
  // Defined in: td/LC32AsmParser.td
  let AssemblyParsers = [LC32AsmParser];
}
