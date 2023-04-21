//===-- LC32ISelLowering.h - LC-3.2 DAG Lowering Interface ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module implements the bulk of lowering. While the DAGToDAG pass handles
// instruction selection, this pass handles the calling convention and stack
// frames.
//
// See: CCState
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H
#define LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H

#include "llvm/CodeGen/TargetLowering.h"

namespace llvm {
class LC32Subtarget;
class LC32RegisterInfo;

// These are all the custom SelectionDAG nodes. Keep this in sync with TableGen.
// Also update getTargetNodeName.
namespace LC32ISD {
enum {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET,
};
} // namespace LC32ISD

class LC32TargetLowering : public TargetLowering {
public:
  explicit LC32TargetLowering(const TargetMachine &TM,
                              const LC32Subtarget &STI);

  const char *getTargetNodeName(unsigned Opcode) const override;

private:
  const LC32RegisterInfo *TRI;

  // Calling Convention
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &dl, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context) const override;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &dl,
                      SelectionDAG &DAG) const override;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H
