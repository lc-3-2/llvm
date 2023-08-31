//===-- LC32SelectionDAGInfo.h - LC-3.2 SelectionDAG Info -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This class is usually used to optimize intrinsics instead of lowering them to
// function calls. For example, `memcpy` would be optimized here. We don't do
// any of that though, so we supply an empty implementation.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LC32_SELECTION_DAG_INFO_H
#define LLVM_LC32_SELECTION_DAG_INFO_H

#include "llvm/CodeGen/SelectionDAGTargetInfo.h"

namespace llvm {

class LC32SelectionDAGInfo : public SelectionDAGTargetInfo {};

} // namespace llvm

#endif // LLVM_LC32_SELECTION_DAG_INFO_H
