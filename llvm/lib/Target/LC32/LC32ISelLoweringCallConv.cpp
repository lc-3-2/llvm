//===-- LC32ISelLoweringCallConv.cpp - LC-3.2 DAG Lowering ------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "LC32MachineFunctionInfo.h"
#include "LC32RegisterInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLoweringCallConv"

// Provides: LC32CallingConv
// Provides: LC32RetCallingConv
#include "LC32GenCallingConv.inc"

// Usually: LowerFormalArguments and LowerCCCArguments
SDValue LC32TargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {

  // We generate the same code for fastcalls and for normal calls
  // Fastcalls are generated for static functions on higher optimizations
  if (CallConv != CallingConv::C && CallConv != CallingConv::Fast)
    report_fatal_error("Unsupported CallConv");

  // Populate variables
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  LC32MachineFunctionInfo *MFnI = MF.getInfo<LC32MachineFunctionInfo>();

  // Initialize the CCState
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, MF, ArgLocs, *DAG.getContext());
  CCInfo.AnalyzeFormalArguments(Ins, LC32CallingConv);

  // If this function is variadic, create a frame index corresponding to the
  // first variadic argument. This will be read by the code that lowers
  // `vastart`.
  if (isVarArg)
    // Note the offset from the frame pointer. Remember that frame index offsets
    // are from the stack pointer on function entry.
    MFnI->VarArgsFI =
        MFI.CreateFixedObject(1, CCInfo.getNextStackOffset(), true);

  // Create frame indicies for each of the arguments
  // See: MSP430ISelLowering.cpp, LanaiISelLowering.cpp
  bool saw_sret = false;
  for (size_t i = 0; i < ArgLocs.size(); i++) {
    const CCValAssign &va = ArgLocs[i];
    assert(i < Ins.size() && "Ins array not big enough");
    assert(va.isMemLoc() && "LC-3.2 should have all arguments on the stack");

    // Check for a struct passed by value
    // In this case, the operand is in memory, with no need to load
    if (Ins[i].Flags.isByVal()) {
      // Get the frame index
      // Note the offset from the frame pointer. Remember that frame index
      // offsets are from the stack pointer on function entry.
      int FI = MFI.CreateFixedObject(Ins[i].Flags.getByValSize(),
                                     va.getLocMemOffset(), true);

      InVals.push_back(DAG.getFrameIndex(FI, MVT::i32));

    } else {
      // Compute the size of the object
      // Notice that we use LocVT here. For i8 and i16, the LocVT is i32 - the
      // size of the register storing the argument. This is because LLVM uses
      // the LocVT with the calling convention to compute offsets.
      assert(va.getLocVT() == MVT::i32 && "LC-3.2 only has i32 registers");

      // Get the frame index
      // Note the offset from the frame pointer. Remember that frame index
      // offsets are from the stack pointer on function entry.
      int FI = MFI.CreateFixedObject(4, va.getLocMemOffset(), true);

      InVals.push_back(DAG.getLoad(
          MVT::i32, dl, Chain, DAG.getFrameIndex(FI, MVT::i32),
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI)));
    }

    // If this operand is to be the pointer for sret, copy it to a register and
    // save it to be returned
    if (Ins[i].Flags.isSRet()) {
      // We can only have one sret
      assert(!saw_sret && "Can only have one sret argument");
      assert(MFnI->SRetAddrReg == 0 && "SRetAddrReg already set");
      saw_sret = true;
      // Create a register to store the return address
      MFnI->SRetAddrReg =
          MF.getRegInfo().createVirtualRegister(&LC32::GPRRegClass);
      // Copy to the register
      Chain = DAG.getCopyToReg(DAG.getEntryNode(), dl, MFnI->SRetAddrReg,
                               InVals[i]);
    }
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
  LC32MachineFunctionInfo *MFnI = MF.getInfo<LC32MachineFunctionInfo>();

  // Initialize CCState
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());
  CCInfo.AnalyzeReturn(Outs, LC32RetCallingConv);

  if (RVLocs.size() == 1) {
    // Just lower onto the return value slot
    assert(Outs.size() == 1 && "Different RVLocs and Outs");
    assert(OutVals.size() == 1 && "Different RVLocs and OutVals");

    // Get the location
    CCValAssign &va = RVLocs[0];
    assert(va.isMemLoc() && "LC-3.2 can only return on the stack");
    assert(va.getLocMemOffset() == 0 && "LC-3.2 has only one return value");

    // Make sure object size is what we expect. We should have it be i32 since
    // those are the only registers we have. I.e. that is our LocVT.
    assert(va.getValVT().getSizeInBits() == 32 &&
           "LC-3.2 only supports words as returns");
    assert(va.getLocInfo() == CCValAssign::Full &&
           "LC-3.2 returns should not require extension");

    // Get the frame index and store
    // Note the offset from the frame pointer. Remember that frame index offsets
    // are from the stack pointer on function entry.
    int FI = MFI.CreateFixedObject(4, -4, false);
    Chain = DAG.getStore(
        Chain, dl, OutVals[0], DAG.getFrameIndex(FI, MVT::i32),
        MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI),
        Align(4));

  } else {
    // This function either returns void or is sret
    assert(RVLocs.empty() && "Should be void or have demoted to sret");
    // If it returns sret, store the pointer onto the stack in place of the
    // return value
    if (MF.getFunction().hasStructRetAttr()) {
      assert(MFnI->SRetAddrReg != 0 && "Should've set register for sret");
      // Copy from the register and store it on the stack
      // Note the offset from the frame pointer. Remember that frame index
      // offsets are from the stack pointer on function entry.
      SDValue v = DAG.getCopyFromReg(Chain, dl, MFnI->SRetAddrReg, MVT::i32);
      int FI = MFI.CreateFixedObject(4, -4, false);
      Chain = DAG.getStore(
          Chain, dl, v, DAG.getFrameIndex(FI, MVT::i32),
          MachinePointerInfo::getFixedStack(DAG.getMachineFunction(), FI),
          Align(4));
    }
  }

  return DAG.getNode(LC32ISD::RET, dl, MVT::Other, Chain);
}

// Usually: LowerCall, LowerCCCCallTo, and LowerCallResult
SDValue LC32TargetLowering::LowerCall(CallLoweringInfo &CLI,
                                      SmallVectorImpl<SDValue> &InVals) const {

  // We generate the same code for fastcalls and for normal calls
  // Fastcalls are generated for static functions on higher optimizations
  if (CLI.CallConv != CallingConv::C && CLI.CallConv != CallingConv::Fast)
    report_fatal_error("Unsupported CallConv");
  // We don't do tail calls
  CLI.IsTailCall = false;

  // Populate variables
  SDValue Chain = CLI.Chain;
  MachineFunction &MF = CLI.DAG.getMachineFunction();

  // Initialize the CCState
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CLI.CallConv, CLI.IsVarArg, MF, ArgLocs,
                 *CLI.DAG.getContext());
  CCInfo.AnalyzeArguments(CLI.Outs, LC32CallingConv);

  // Find out how many bytes to push onto the stack
  unsigned NumBytes = CCInfo.getNextStackOffset();
  assert(this->getFrameIndexTy(CLI.DAG.getDataLayout()) == MVT::i32 &&
         "LC-3.2 pointers should be i32");

  Chain = CLI.DAG.getCALLSEQ_START(Chain, NumBytes, 0, CLI.DL);

  // Store the arguments onto the stack
  // See: MSP430ISelLowering.cpp
  SDValue sp;
  SmallVector<SDValue, 12> arg_store_chains;
  for (size_t i = 0; i < ArgLocs.size(); i++) {
    CCValAssign &va = ArgLocs[i];
    SDValue &arg = CLI.OutVals[i];

    // We shouldn't require extension because we only have 32-bit registers
    assert(va.isMemLoc() && "LC-3.2 arguments should be on the stack");
    assert(va.getLocInfo() == CCValAssign::Full &&
           "LC-3.2 arguments should not require extension");

    // Initialize the SP on the first pass
    if (!sp.getNode())
      sp = CLI.DAG.getCopyFromReg(Chain, CLI.DL, LC32::SP, MVT::i32);

    // Compute the address
    SDValue arg_store_addr = CLI.DAG.getNode(
        ISD::ADD, CLI.DL, MVT::i32, sp,
        CLI.DAG.getConstant(va.getLocMemOffset(), CLI.DL, MVT::i32));

    // Do the store
    // May have to convert to memcpy if passing struct by value. Make sure this
    // is always inlined, otherwise we get nested call frame operations.
    SDValue arg_store;
    ISD::ArgFlagsTy flgs = CLI.Outs[i].Flags;
    if (flgs.isByVal()) {
      arg_store = CLI.DAG.getMemcpy(
          Chain, CLI.DL, arg_store_addr, arg,
          CLI.DAG.getConstant(flgs.getByValSize(), CLI.DL, MVT::i32),
          flgs.getNonZeroByValAlign(), false, true, false, MachinePointerInfo(),
          MachinePointerInfo());
    } else {
      arg_store = CLI.DAG.getStore(Chain, CLI.DL, arg, arg_store_addr,
                                   MachinePointerInfo());
    }

    // Add the chain
    arg_store_chains.push_back(arg_store);
  }

  // Create a token of all the argument stores
  if (!arg_store_chains.empty())
    Chain = CLI.DAG.getTokenFactor(CLI.DL, arg_store_chains);

  // Do the call
  Chain = CLI.DAG.getNode(LC32ISD::CALL, CLI.DL,
                          CLI.DAG.getVTList(MVT::Other, MVT::Glue), Chain,
                          CLI.Callee);

  // Analyze the return values
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCRetInfo(CLI.CallConv, CLI.IsVarArg, MF, RVLocs,
                    *CLI.DAG.getContext());
  CCRetInfo.AnalyzeCallResult(CLI.Ins, LC32RetCallingConv);

  if (RVLocs.size() == 1) {
    // Read from the return value slot
    assert(CLI.Ins.size() == 1 && "Different RVLocs and Ins");

    // Get the location
    CCValAssign &va = RVLocs[0];
    assert(va.isMemLoc() && "LC-3.2 can only return on the stack");
    assert(va.getLocMemOffset() == 0 && "LC-3.2 has only one return value");

    // Make sure object size is what we expect
    assert(va.getValVT().getSizeInBits() == 32 &&
           "LC-3.2 only supports words as returns");

    // Get the stack pointer and load from it
    Chain = CLI.DAG.getCopyFromReg(Chain.getValue(0), CLI.DL, LC32::SP,
                                   MVT::i32, Chain.getValue(1));
    Chain = CLI.DAG.getLoad(MVT::i32, CLI.DL, Chain.getValue(1),
                            Chain.getValue(0), MachinePointerInfo());

    // Create a virtual register and store the load in it
    Register reg_ret =
        MF.getRegInfo().createVirtualRegister(&LC32::GPRRegClass);
    Chain = CLI.DAG.getCopyToReg(Chain.getValue(1), CLI.DL, reg_ret,
                                 Chain.getValue(0), SDValue());

    // Teardown the stack
    // Remember to pop the return value too
    Chain = CLI.DAG.getCALLSEQ_END(Chain.getValue(0), NumBytes, 4,
                                   Chain.getValue(1), CLI.DL);
    // Return the value
    Chain = CLI.DAG.getCopyFromReg(Chain.getValue(0), CLI.DL, reg_ret, MVT::i32,
                                   Chain.getValue(1));
    InVals.push_back(Chain.getValue(0));
    return Chain.getValue(1);

  } else {
    // This function either returns void or is sret
    // Either way, just return
    assert(RVLocs.empty() && "Should be void or have demoted to sret");
    return CLI.DAG.getCALLSEQ_END(Chain, NumBytes, 4, Chain.getValue(1),
                                  CLI.DL);
  }
}
