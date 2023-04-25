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

  // Setup register classes
  this->TRI = STI.getRegisterInfo();
  this->addRegisterClass(MVT::i32, &LC32::GPRRegClass);
  this->computeRegisterProperties(this->TRI);

  // Setup VLA
  this->setStackPointerRegisterToSaveRestore(LC32::SP);

  // Setup booleans
  this->setBooleanContents(ZeroOrOneBooleanContent);
  this->setBooleanVectorContents(ZeroOrOneBooleanContent);
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
