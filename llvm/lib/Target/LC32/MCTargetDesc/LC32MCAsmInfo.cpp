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

void LC32MCAsmInfo::anchor() {}

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
