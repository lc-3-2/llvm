//===-- LC32MCObjectFileInfo.h - LC-3.2 Object File Info -------*- C++ -*--===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32MCObjectFileInfo.h"
using namespace llvm;
#define DEBUG_TYPE "LC32MCObjectFileInfo"

unsigned LC32MCObjectFileInfo::getTextSectionAlignment() const { return 2; }
