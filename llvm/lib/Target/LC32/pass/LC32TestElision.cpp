//==- LC32TestElision.cpp - Test Elision Pass for LC-3.2 --------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "pass/LC32TestElision.h"
using namespace llvm;
#define DEBUG_TYPE "LC32TestElision"

char LC32TestElision::ID = 0;

INITIALIZE_PASS(LC32TestElision, DEBUG_TYPE, "LC-3.2 Test Elision", false,
                false)

bool LC32TestElision::runOnMachineFunction(MachineFunction &MF) {
  return false;
}
