//==- LC32FrameLowering.cpp - Define Frame Lowering for LC-3.2 --*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32FrameLowering.h"
using namespace llvm;
#define DEBUG_TYPE "LC32FrameLowering"

LC32FrameLowering::LC32FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), -12,
                          Align(4)) {}

void LC32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  llvm_unreachable("UNIMPLEMENTED");
}

void LC32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  llvm_unreachable("UNIMPLEMENTED");
}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }
