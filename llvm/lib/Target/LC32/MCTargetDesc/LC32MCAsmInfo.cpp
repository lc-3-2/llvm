//===-- LC32MCAsmInfo.cpp - LC-3.2 Asm Properties --------------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32MCAsmInfo.h"
using namespace llvm;
#define DEBUG_TYPE "LC32MCAsmInfo"

LC32MCAsmInfo::LC32MCAsmInfo(const Triple &TT, const MCTargetOptions &Options) {

  // Pseudo instructions reach a length of 12 bytes
  // All instructions are halfword aligned
  this->MaxInstLength = 12;
  this->MinInstAlignment = 2;

  // Comments are semicolons
  // We can't allow usual comments because # denotes literals
  this->CommentString = ";";
  this->AllowAdditionalComments = false;

  // Use the .align directive
  this->UseDotAlignForAlignment = true;

  // Directive aliases
  this->ZeroDirective = "\t.blkb\t";
  this->ZeroDirectiveSupportsNonZeroValue = false;
  this->AsciiDirective = "\t.string\t";
  this->AscizDirective = "\t.stringz\t";
  this->GlobalDirective = "\t.global\t";
  this->Data8bitsDirective = "\t.fillb\t";
  this->Data16bitsDirective = "\t.fillh\t";
  this->Data32bitsDirective = "\t.fillw\t";
  this->Data64bitsDirective = "\t.fillq\t";

  // Emit debug information
  this->SupportsDebugInformation = true;
}

bool LC32MCAsmInfo::isValidUnquotedName(StringRef Name) const {
  bool ret = true;
  // Check register names
  ret &= Name.upper() != "R0";
  ret &= Name.upper() != "R1";
  ret &= Name.upper() != "R2";
  ret &= Name.upper() != "R3";
  ret &= Name.upper() != "R4";
  ret &= Name.upper() != "R5";
  ret &= Name.upper() != "R6";
  ret &= Name.upper() != "R7";
  ret &= Name.upper() != "GP";
  ret &= Name.upper() != "FP";
  ret &= Name.upper() != "SP";
  ret &= Name.upper() != "LR";
  // Check hex immediates
  ret &= !Name.startswith("x");
  ret &= !Name.startswith("X");
  // Return
  return ret;
}
