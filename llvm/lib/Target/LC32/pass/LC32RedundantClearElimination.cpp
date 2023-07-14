//==- LC32RedundantClearElimination.cpp - Remove Register Clears *- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32CLOpts.h"
#include "pass/LC32RedundantClearElimination.h"
#include "llvm/ADT/Statistic.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32RedundantClearElimination"

char LC32RedundantClearElimination::ID = 0;

INITIALIZE_PASS(LC32RedundantClearElimination, DEBUG_TYPE,
                "LC-3.2 Redundant Register Clear Elimination", false, false)

STATISTIC(NumClearsEliminated,
          "Number of redundant register clears eliminated");

bool LC32RedundantClearElimination::runOnMachineFunction(
    MachineFunction &MF) {
  // TODO
  return false;
}
