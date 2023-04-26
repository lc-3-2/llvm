//===-- LC32ISelLowering.cpp - LC-3.2 DAG Lowering Interface ----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLowering"

LC32TargetLowering::LC32TargetLowering(const TargetMachine &TM,
                                       const LC32Subtarget &STI)
    : TargetLowering(TM) {

  // Convenience lists
  static const std::vector<MVT> INT_TYPES_LTI8 = {MVT::i1, MVT::i4};
  static const std::vector<MVT> INT_TYPES_LTI32 = {MVT::i1, MVT::i4, MVT::i8,
                                                   MVT::i16};
  static const std::vector<MVT> INT_TYPES_I8_I16 = {MVT::i8, MVT::i16};

  // Setup register classes
  this->TRI = STI.getRegisterInfo();
  this->addRegisterClass(MVT::i32, &LC32::GPRRegClass);
  this->computeRegisterProperties(this->TRI);

  // Setup VLA
  this->setStackPointerRegisterToSaveRestore(LC32::SP);

  // Setup booleans
  this->setBooleanContents(ZeroOrOneBooleanContent);
  this->setBooleanVectorContents(ZeroOrOneBooleanContent);

  // Set function alignment
  this->setMinFunctionAlignment(Align(2));
  this->setPrefFunctionAlignment(Align(4));

  // Set stack alignment
  this->setMinStackArgumentAlignment(Align(4));

  // Setup how we should extend loads
  // Not all loads have corresponding instructions
  for (const auto &vt : INT_TYPES_LTI8) {
    this->setLoadExtAction(ISD::EXTLOAD, MVT::i32, vt, Promote);
    this->setLoadExtAction(ISD::SEXTLOAD, MVT::i32, vt, Promote);
    this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, vt, Promote);
  }
  for (const auto &vt : INT_TYPES_I8_I16) {
    this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, vt, Expand);
  }

  // Promote smaller multiplications and divisions to 32-bit
  for (const auto &vt : INT_TYPES_LTI32) {
    this->setOperationAction(ISD::MUL, vt, Promote);
    this->setOperationAction(ISD::MULHU, vt, Promote);
    this->setOperationAction(ISD::MULHS, vt, Promote);
    this->setOperationAction(ISD::SMUL_LOHI, vt, Promote);
    this->setOperationAction(ISD::UMUL_LOHI, vt, Promote);
    this->setOperationAction(ISD::SDIV, vt, Promote);
    this->setOperationAction(ISD::UDIV, vt, Promote);
    this->setOperationAction(ISD::SREM, vt, Promote);
    this->setOperationAction(ISD::UREM, vt, Promote);
    this->setOperationAction(ISD::SDIVREM, vt, Promote);
    this->setOperationAction(ISD::UDIVREM, vt, Promote);
  }
  // Do libcalls for 32-bit multiplications and divisions
  this->setOperationAction(ISD::MUL, MVT::i32, LibCall);
  this->setOperationAction(ISD::MULHU, MVT::i32, Expand);
  this->setOperationAction(ISD::MULHS, MVT::i32, Expand);
  this->setOperationAction(ISD::SMUL_LOHI, MVT::i32, Expand);
  this->setOperationAction(ISD::UMUL_LOHI, MVT::i32, Expand);
  this->setOperationAction(ISD::SDIV, MVT::i32, LibCall);
  this->setOperationAction(ISD::UDIV, MVT::i32, LibCall);
  this->setOperationAction(ISD::SREM, MVT::i32, LibCall);
  this->setOperationAction(ISD::UREM, MVT::i32, LibCall);
  this->setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  this->setOperationAction(ISD::UDIVREM, MVT::i32, Expand);

  // Bitwise operations only work on 32-bit
  for (const auto &vt : INT_TYPES_LTI32) {
    this->setOperationAction(ISD::AND, vt, Promote);
    this->setOperationAction(ISD::OR, vt, Promote);
    this->setOperationAction(ISD::XOR, vt, Promote);
  }

  // Promote smaller shifts
  for (const auto &vt : INT_TYPES_LTI32) {
    this->setOperationAction(ISD::SHL, vt, Promote);
    this->setOperationAction(ISD::SRA, vt, Promote);
    this->setOperationAction(ISD::SRL, vt, Promote);
  }

  // Expand rotations and part shifts
  for (const auto &vt : MVT::integer_valuetypes()) {
    this->setOperationAction(ISD::ROTL, vt, Expand);
    this->setOperationAction(ISD::ROTR, vt, Expand);
    this->setOperationAction(ISD::SHL_PARTS, vt, Expand);
    this->setOperationAction(ISD::SRA_PARTS, vt, Expand);
    this->setOperationAction(ISD::SRL_PARTS, vt, Expand);
  }

  // Extension doesn't have custom instructions
  for (const auto &vt : MVT::integer_valuetypes()) {
    this->setOperationAction(ISD::SIGN_EXTEND, vt, Expand);
    this->setOperationAction(ISD::ZERO_EXTEND, vt, Expand);
    this->setOperationAction(ISD::ANY_EXTEND, vt, Expand);
    this->setOperationAction(ISD::TRUNCATE, vt, Expand);
    this->setOperationAction(ISD::SIGN_EXTEND_INREG, vt, Expand);
  }

  // Counting bits doesn't have custom instructions either
  for (const auto &vt : MVT::integer_valuetypes()) {
    this->setOperationAction(ISD::BSWAP, vt, Expand);
    this->setOperationAction(ISD::CTTZ, vt, Expand);
    this->setOperationAction(ISD::CTLZ, vt, Expand);
    this->setOperationAction(ISD::CTPOP, vt, Expand);
    this->setOperationAction(ISD::BITREVERSE, vt, Expand);
    this->setOperationAction(ISD::PARITY, vt, Expand);
    this->setOperationAction(ISD::CTTZ_ZERO_UNDEF, vt, Expand);
    this->setOperationAction(ISD::CTLZ_ZERO_UNDEF, vt, Expand);
  }
}

const char *LC32TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case LC32ISD::RET:
    return "LC32ISD::RET";
  }
  return nullptr;
}

// Provides: LC32CallingConv
// Provides: LC32RetCallingConv
#include "LC32GenCallingConv.inc"

// Usually: LowerFormalArguments and LowerCCCArguments
SDValue LC32TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  // We only support the one convention
  if (CallConv != CallingConv::C)
    report_fatal_error("Unsupported CallConv");

  // Populate variables
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Initialize the CCState
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, LC32CallingConv);

  // Create frame indicies for each of the arguments
  // See: LanaiISelLowering.cpp
  for (const CCValAssign &va : ArgLocs) {
    assert(va.isMemLoc() && "LC-3.2 should have all arguments on the stack");

    // Make sure object size is what we expect
    assert(va.getValVT().getSizeInBits() == 32 &&
           "LC-3.2 only supports words as arguments");

    // Get the frame index
    // Note the offset from the frame pointer
    int FI = MFI.CreateFixedObject(4, 16 + va.getLocMemOffset(), true);
    InVals.push_back(DAG.getLoad(
        va.getLocVT(), dl, Chain, DAG.getFrameIndex(FI, MVT::i32),
        MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI),
        Align(4)));
  }

  return Chain;
}

bool LC32TargetLowering::CanLowerReturn(
    CallingConv::ID CallConv, MachineFunction &MF, bool isVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context) const {

  // Initialize CCState
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, MF, RVLocs, Context);

  // Process
  // The return value controls if sret demotion is performed.
  return CCInfo.CheckReturn(Outs, LC32RetCallingConv);
}

SDValue
LC32TargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                                bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &dl, SelectionDAG &DAG) const {

  // Populate variables
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();

  // Initialize CCState
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, LC32RetCallingConv);

  // If we got here, it means we aren't demoting to sret
  // Therefore, just lower onto the return value slot

  if (RVLocs.size() == 1) {
    assert(Outs.size() == 1 && "More RVLocs than Outs");
    assert(OutVals.size() == 1 && "More RVLocs than OutVals");

    // Get the location
    CCValAssign &va = RVLocs[0];
    assert(va.isMemLoc() && "LC-3.2 can only return on the stack");
    assert(va.getLocMemOffset() == 0 && "LC-3.2 has only one return value");

    // Make sure object size is what we expect
    assert(va.getValVT().getSizeInBits() == 32 &&
           "LC-3.2 only supports words as returns");

    // Get the frame index and store
    int FI = MFI.CreateFixedObject(4, 12, false);
    Chain = DAG.getStore(
        Chain, dl, OutVals[0], DAG.getFrameIndex(FI, MVT::i32),
        MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI),
        Align(4));

  } else {
    assert(RVLocs.empty() && "Should have demoted to sret");
  }

  SmallVector<SDValue, 4> RetOps(1, Chain);
  return DAG.getNode(LC32ISD::RET, dl, MVT::Other, RetOps);
}
