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
// For the required stuff, LC32ISelLoweringConstructor.cpp houses the
// constructor. LC32ISelLoweringCallConv.cpp handles calling convention
// lowering. Finally, LC32ISelLoweringOps.cpp handles operations that have to be
// lowered custom.
//
// There is also optional stuff. LC32ISelLoweringInlineAsm.cpp handles
// constraints for inline assembly. The inline assembly was mostly copied from
// AVR. LC32ISelLoweringConstructor.cpp also houses functions that control how
// the optimizer lowers code.
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
  LOWERING_NOT,

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

  // Output 0: Chain
  // Output 1: Value
  // Operand 0: Chain
  // Operand 1: NZP
  // Operand 2: Value to compare with zero
  // Operand 3: Value if true
  // Operand 4: Value if false
  SELECT_CMP_ZERO,

  // Output 0: Address
  // Operand 0: Address
  ADDR_WRAPPER,
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

  // This delegates to combine the DAG
  // See: LC32ISelLoweringOps.cpp
  SDValue PerformDAGCombine(SDNode *N, DAGCombinerInfo &DCI) const override;

  // This emits instructions that need help
  // See: LC32ISelLoweringOps.cpp
  MachineBasicBlock *
  EmitInstrWithCustomInserter(MachineInstr &MI,
                              MachineBasicBlock *BB) const override;

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
  SDValue LowerAddress(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSUB(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerOR(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerBR_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerSELECT_CC(SDValue Op, SelectionDAG &DAG) const;
  SDValue LowerVASTART(SDValue Op, SelectionDAG &DAG) const;

  // See: LC32ISelLoweringOps.cpp
  SDValue visitXOR(SDNode *N, DAGCombinerInfo &DCI) const;
  SDValue visitLOWERING_NOT(SDNode *N, DAGCombinerInfo &DCI) const;

  // See: LC32ISelLoweringOps.cpp
  MachineBasicBlock *emitC_SELECT_CMP_ZERO(MachineInstr &MI,
                                           MachineBasicBlock *BB) const;

  // See LC32ISelLoweringInlineAsm.cpp
  ConstraintType getConstraintType(StringRef Constraint) const override;
  ConstraintWeight
  getSingleConstraintMatchWeight(AsmOperandInfo &info,
                                 const char *constraint) const override;
  std::pair<unsigned, const TargetRegisterClass *>
  getRegForInlineAsmConstraint(const TargetRegisterInfo *RI,
                               StringRef Constraint, MVT VT) const override;
  void LowerAsmOperandForConstraint(SDValue Op, std::string &Constraint,
                                    std::vector<SDValue> &Ops,
                                    SelectionDAG &DAG) const override;
  Register getRegisterByName(const char *RegName, LLT Ty,
                             const MachineFunction &MF) const override;

  // See: LC32ISelLoweringConstructor.cpp
  bool useSoftFloat() const override;
  bool isSelectSupported(SelectSupportKind kind) const override;
  bool reduceSelectOfFPConstantLoads(EVT CmpOpVT) const override;
  bool preferZeroCompareBranch() const override;
  bool hasBitPreservingFPLogic(EVT VT) const override;
  bool convertSetCCLogicToBitwiseLogic(EVT VT) const override;
  bool hasAndNotCompare(SDValue Y) const override;
  bool hasAndNot(SDValue X) const override;
  bool shouldFoldMaskToVariableShiftPair(SDValue X) const override;
  bool shouldFoldConstantShiftPairToMask(const SDNode *N,
                                         CombineLevel Level) const override;
  bool shouldTransformSignedTruncationCheck(EVT XVT,
                                            unsigned KeptBits) const override;
  bool shouldNormalizeToSelectSequence(LLVMContext &Context,
                                       EVT VT) const override;
  bool convertSelectOfConstantsToMath(EVT VT) const override;
  bool decomposeMulByConstant(LLVMContext &Context, EVT VT,
                              SDValue C) const override;
  bool isMulAddWithConstProfitable(SDValue AddNode,
                                   SDValue ConstNode) const override;
  bool isLegalICmpImmediate(int64_t Value) const override;
  bool isLegalAddImmediate(int64_t Value) const override;

  bool isDesirableToCommuteWithShift(const SDNode *N,
                                     CombineLevel Level) const override;
  bool isDesirableToTransformToIntegerOp(unsigned Opc, EVT VT) const override;

  /**
   * Helper method for LowerBR_CC and LowerSELECT_CC.
   *
   * Both those operations need to be lowered to a comparison against zero.
   * This emits a structure with the results.
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
