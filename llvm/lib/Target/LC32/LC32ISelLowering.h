//===-- LC32ISelLowering.h - LC-3.2 DAG Lowering Interface ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module tells other passes what operations are legal. It also provides
// custom lowering operations as needed.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H
#define LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class LC32Subtarget;

class LC32TargetLowering : public TargetLowering {
public:
  explicit LC32TargetLowering(const TargetMachine &TM,
                              const LC32Subtarget &STI);
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H
