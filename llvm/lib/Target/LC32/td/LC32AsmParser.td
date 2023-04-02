//===- LC32AsmParser.td - Assembly Parser Customization ----*- tablegen -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// We use this file to customize how the assembly parser is generated.
// Specifically, we enable register alias matching, which is off by default.
//
//===----------------------------------------------------------------------===//

def LC32AsmParser : AsmParser {
  let ShouldEmitMatchRegisterAltName = true;
}
