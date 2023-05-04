//===-- LC32ISelLowering.h - LC-3.2 DAG Lowering Interface ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module implements the bulk of lowering. It handles the calling
// convention, as well as custom operations. The constructor tells the framework
// what custom operations need lowering. Some of these operations need library
// calls. The list of library calls is in llvm/IR/RuntimeLibcalls.def.
//
// Because of the sheer size of this module, it is split over multiple files.
// LC32ISelLoweringConstructor.cpp houses the constructor.
// LC32ISelLoweringCallConv.cpp handles calling convention lowering. Finally,
// LC32ISelLoweringOps.cpp handles operations that have to be lowered custom.
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

  // Behaves just like NOT
  // See: td/instr/LC32ALUInstrInfo.td
  OR_LOWERING_NOT,

  // Output 0: Chain
  // Output 1: Glue
  // Operand 0: Chain
  // Operand 1: Callee
  // Operand 2: Glue
  // See: td/instr/LC32SubroutineInstrInfo.tfd
  CALL,

  // Operand 0: Chain
  RET,

  // Output 0: Chain
  // Operand 0: Chain
  // Operand 1: NZP
  // Operand 2: Value to compare with zero
  // Operand 3: Target
  BR_CMP_ZERO,
};
} // namespace LC32ISD

class LC32TargetLowering : public TargetLowering {
public:
  // See: LC32ISelLoweringConstructor.cpp
  LC32TargetLowering(const TargetMachine &TM, const LC32Subtarget &STI);

  // See: LC32ISelLoweringOps.cpp
  const char *getTargetNodeName(unsigned Opcode) const override;

  // This delegates to different functions to custom lower operations
  // See: LC32ISelLoweringOps.cpp
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;

  // This deleates to combine the DAG
  // See: LC32ISelLoweringOps.cpp
  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

private:
  const LC32RegisterInfo *TRI;

  // Calling Convention
  // See: LC32ISelLoweringCallConv.cpp
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
  SDValue LowerCall(CallLoweringInfo &CLI,
                    SmallVectorImpl<SDValue> &InVals) const override;

  // See: LC32ISelLoweringOps.cpp
  SDValue LowerOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;

  // See: LC32ISelLoweringOps.cpp
  SDValue visitXOR(SDNode *N, DAGCombinerInfo &DCI) const;

  /**
   * Helper method for LowerBR_CC and LowerSELECT_CC.
   *
   * Both those operations need to be lowered to a comparison against zero. This
   * emits a structure with the results.
   */
  struct DoCMPResult {
    SDValue Chain; //< The new chain
    SDValue NZP;   //< The NZP to put for the branch
    SDValue Value; //< The value to compare against zero
  };
  DoCMPResult DoCMP(SelectionDAG &DAG, SDLoc dl, SDValue Chain,
                    ISD::CondCode CC, SDValue LHS, SDValue RHS) const;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32ISELLOWERING_H
