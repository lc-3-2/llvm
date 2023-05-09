//===-- LC32ISelLoweringOps.cpp - LC-3.2 DAG Lowering Interface -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/Support/CommandLine.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLoweringOps"

static cl::opt<bool> UseSignedCMPLibCall(
    "lc_3.2-use-libcall-for-signed-cmp",
    cl::desc("When comparing signed integers, use a libcall to prevent "
             "overflow instead of just subtracting"),
    cl::init(false));
static cl::opt<bool> UseUnsignedCMPLibCall(
    "lc_3.2-use-libcall-for-unsigned-cmp",
    cl::desc("When comparing unsigned integers, use a libcall to prevent "
             "overflow instead of just subtracting"),
    cl::init(false));

static cl::opt<bool>
    UseCMPLibCall("lc_3.2-use-libcall-for-cmp",
                  cl::desc("Set --lc_3.2-use-libcall-for-signed-cmp and "
                           "--lc_3.2-use-libcall-for-unsigned-cmp"),
                  cl::init(false), cl::callback([&](const bool &v) {
                    UseSignedCMPLibCall = v;
                    UseUnsignedCMPLibCall = v;
                  }));

static cl::opt<std::string> SignedCMPLibCallName(
    "lc_3.2-signed-cmp-libcall-name",
    cl::desc("What function to call when comparing signed integers in the "
             "presence of --lc_3.2-use-libcall-for-signed-cmp"),
    cl::init("__cmpsi3"), cl::Hidden);
static cl::opt<std::string> UnsignedCMPLibCallName(
    "lc_3.2-unsigned-cmp-libcall-name",
    cl::desc("What function to call when comparing unsigned integers in the "
             "presence of --lc_3.2-use-libcall-for-unsigned-cmp"),
    cl::init("__ucmpsi3"), cl::Hidden);

const char *LC32TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case LC32ISD::LOWERING_NOT:
    return "LC32ISD::LOWERING_NOT";
  case LC32ISD::CALL:
    return "LC32ISD::CALL";
  case LC32ISD::RET:
    return "LC32ISD::RET";
  case LC32ISD::BR_CMP_ZERO:
    return "LC32ISD::CMP_ZERO";
  case LC32ISD::SELECT_CMP_ZERO:
    return "LC32ISD::SELECT_CMP_ZERO";
  default:
    return nullptr;
  }
}

SDValue LC32TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::SUB:
    return this->LowerSUB(Op, DAG);
  case ISD::OR:
    return this->LowerOR(Op, DAG);
  case ISD::BR_CC:
    return this->LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:
    return this->LowerSELECT_CC(Op, DAG);
  default:
    llvm_unreachable("Bad opcode for custom lowering");
  }
}

SDValue LC32TargetLowering::PerformDAGCombine(SDNode *N,
                                              DAGCombinerInfo &DCI) const {
  switch (N->getOpcode()) {
  case ISD::XOR:
    return this->visitXOR(N, DCI);
  case LC32ISD::LOWERING_NOT:
    return this->visitLOWERING_NOT(N, DCI);
  case LC32ISD::CALL:
  case LC32ISD::RET:
  case LC32ISD::BR_CMP_ZERO:
  case LC32ISD::SELECT_CMP_ZERO:
    return SDValue();
  default:
    llvm_unreachable("Bad opcode for custom combine");
  }
}

MachineBasicBlock *
LC32TargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                MachineBasicBlock *BB) const {
  switch (MI.getOpcode()) {
  case LC32::C_SELECT_CMP_ZERO:
    return this->emitC_SELECT_CMP_ZERO(MI, BB);
  default:
    llvm_unreachable("Bad opcode for custom insertion");
  }
}

SDValue LC32TargetLowering::LowerSUB(SDValue Op, SelectionDAG &DAG) const {
  // Populate variables
  SDLoc dl(Op);

  // Check the type
  // This is fine since this happens after type legalization
  assert(Op.getValueType() == MVT::i32 && "Only i32 is supported for SUB");
  assert(Op.getOperand(0).getValueType() == MVT::i32 &&
         "Only i32 is supported for SUB");
  assert(Op.getOperand(1).getValueType() == MVT::i32 &&
         "Only i32 is supported for SUB");

  // Flip the bits and add one
  // Remember to inhibit combining on the NOT
  // See: td/instr/LC32ALUInstrInfo.td
  SDValue b_prime =
      DAG.getNode(LC32ISD::LOWERING_NOT, dl, MVT::i32, Op.getOperand(1));
  SDValue b_neg = DAG.getNode(ISD::ADD, dl, MVT::i32, b_prime,
                              DAG.getConstant(1, dl, MVT::i32));
  return DAG.getNode(ISD::ADD, dl, MVT::i32, Op.getOperand(0), b_neg);
}

SDValue LC32TargetLowering::LowerOR(SDValue Op, SelectionDAG &DAG) const {
  // Populate variables
  SDLoc dl(Op);

  // Check the type
  // This is fine since this happens after type legalization
  assert(Op.getValueType() == MVT::i32 && "Only i32 is supported for OR");
  assert(Op.getOperand(0).getValueType() == MVT::i32 &&
         "Only i32 is supported for OR");
  assert(Op.getOperand(1).getValueType() == MVT::i32 &&
         "Only i32 is supported for OR");

  // Use DeMorgan's law to expand
  // Remember to inhibit combining on the top-level node
  // See: td/instr/LC32ALUInstrInfo.td
  SDValue a_prime = DAG.getNOT(dl, Op.getOperand(0), MVT::i32);
  SDValue b_prime = DAG.getNOT(dl, Op.getOperand(1), MVT::i32);
  SDValue x_prime = DAG.getNode(ISD::AND, dl, MVT::i32, a_prime, b_prime);
  return DAG.getNode(LC32ISD::LOWERING_NOT, dl, MVT::i32, x_prime);
}

SDValue LC32TargetLowering::visitXOR(SDNode *N, DAGCombinerInfo &DCI) const {
  // fold (not (N_LOWERING_NOT x)) -> x
  if (N->getOperand(0).getNode() != nullptr &&
      N->getOperand(0).getOpcode() == LC32ISD::LOWERING_NOT &&
      isAllOnesConstant(N->getOperand(1))) {
    return N->getOperand(0).getNode()->getOperand(0);
  }
  // Can't combine here
  return SDValue();
}

SDValue LC32TargetLowering::visitLOWERING_NOT(SDNode *N,
                                              DAGCombinerInfo &DCI) const {
  // fold (N_LOWERING_NOT (not x)) -> x
  if (N->getOperand(0).getNode() != nullptr &&
      N->getOperand(0).getOpcode() == ISD::XOR &&
      isAllOnesConstant(N->getOperand(0).getNode()->getOperand(1))) {
    return N->getOperand(0).getNode()->getOperand(0);
  }
  // Can't combine here
  return SDValue();
}

SDValue LC32TargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  // Populate variables
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS = Op.getOperand(2);
  SDValue RHS = Op.getOperand(3);
  SDValue Target = Op.getOperand(4);
  SDLoc dl(Op);
  // Generate the comparison
  DoCMPResult cmp_res = this->DoCMP(DAG, dl, Chain, CC, LHS, RHS);
  // Generate the branch
  return DAG.getNode(LC32ISD::BR_CMP_ZERO, dl, Op.getValueType(), cmp_res.Chain,
                     cmp_res.NZP, cmp_res.Value, Target);
}

SDValue LC32TargetLowering::LowerSELECT_CC(SDValue Op,
                                           SelectionDAG &DAG) const {
  // Populate variables
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  SDValue TrueV = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc dl(Op);
  // Check the type
  // This is fine since this happens after type legalization
  assert(Op.getValueType() == MVT::i32 &&
         "Only i32 is supported for SELECT_CC");
  // Generate the comparison
  DoCMPResult cmp_res = this->DoCMP(DAG, dl, DAG.getEntryNode(), CC, LHS, RHS);
  // Generate the selection
  return DAG.getNode(LC32ISD::SELECT_CMP_ZERO, dl, MVT::i32, cmp_res.Chain,
                     cmp_res.NZP, cmp_res.Value, TrueV, FalseV);
}

MachineBasicBlock *
LC32TargetLowering::emitC_SELECT_CMP_ZERO(MachineInstr &MI,
                                          MachineBasicBlock *BB) const {

  // Populate variables
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  MachineFunction *MF = BB->getParent();
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  DebugLoc dl = MI.getDebugLoc();

  // We create this pattern
  //  StartBB:
  //    %truev = ...
  //    %falsev = ...
  //    C_BR_CMP_ZERO $nzp, %value, TrueBB
  //    BR 7, FalseBB
  //  TrueBB:
  //    BR 7, RejoinBB
  //  FalseBB:
  //    BR 7, RejoinBB
  //  RejoinBB:
  //    %result = phi [%truev, TrueBB], [%falsev, FalseBB]

  // Populate variables for the start block
  MachineBasicBlock *start_mbb = BB;
  // Create the new basic blocks
  // These correspond to the same IR basic block as the original SELECT_CC
  MachineBasicBlock *true_mbb = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *false_mbb = MF->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *rejoin_mbb = MF->CreateMachineBasicBlock(LLVM_BB);
  // Insert the new basic blocks
  MachineFunction::iterator insert_point = ++BB->getIterator();
  MF->insert(insert_point, true_mbb);
  MF->insert(insert_point, false_mbb);
  MF->insert(insert_point, rejoin_mbb);

  // Transfer everything after the SELECT to the rejoin block
  rejoin_mbb->splice(rejoin_mbb->begin(), start_mbb,
                     std::next(MachineBasicBlock::iterator(MI)),
                     start_mbb->end());
  rejoin_mbb->transferSuccessorsAndUpdatePHIs(start_mbb);

  // Do branches on the start block
  start_mbb->addSuccessor(true_mbb);
  start_mbb->addSuccessor(false_mbb);
  BuildMI(start_mbb, dl, TII.get(LC32::C_BR_CMP_ZERO))
      .addImm(MI.getOperand(1).getImm())
      .addReg(MI.getOperand(2).getReg())
      .addMBB(true_mbb);
  BuildMI(start_mbb, dl, TII.get(LC32::BR)).addImm(0b111).addMBB(false_mbb);

  // Handle true block
  true_mbb->addSuccessor(rejoin_mbb);
  BuildMI(true_mbb, dl, TII.get(LC32::BR)).addImm(0b111).addMBB(rejoin_mbb);
  // Handle false block
  false_mbb->addSuccessor(rejoin_mbb);
  BuildMI(false_mbb, dl, TII.get(LC32::BR)).addImm(0b111).addMBB(rejoin_mbb);

  // Handle rejoin
  BuildMI(*rejoin_mbb, rejoin_mbb->begin(), dl, TII.get(LC32::PHI),
          MI.getOperand(0).getReg())
      .addReg(MI.getOperand(3).getReg())
      .addMBB(true_mbb)
      .addReg(MI.getOperand(4).getReg())
      .addMBB(false_mbb);

  // Delete the pseudo instruction and return
  MI.eraseFromParent();
  return rejoin_mbb;
}

LC32TargetLowering::DoCMPResult
LC32TargetLowering::DoCMP(SelectionDAG &DAG, SDLoc dl, SDValue Chain,
                          ISD::CondCode CC, SDValue LHS, SDValue RHS) const {
  // Check that comparisons have the right type
  assert(LHS.getValueType() == MVT::i32 &&
         "Only i32 is supported for comparison");
  assert(RHS.getValueType() == MVT::i32 &&
         "Only i32 is supported for comparison");

  switch (CC) {
  // For equality, we can use XOR
  case ISD::SETEQ:
  case ISD::SETNE: {
    // Compute the nzp
    uint8_t nzp;
    if (CC == ISD::SETEQ)
      nzp = 0b010;
    if (CC == ISD::SETNE)
      nzp = 0b101;
    // Return
    return DoCMPResult{
        Chain,
        DAG.getTargetConstant(nzp, dl, MVT::i32),
        DAG.getNode(ISD::XOR, dl, MVT::i32, LHS, RHS),
    };
  }

  // For inequality, we either subtract or do a libcall
  case ISD::SETLT:
  case ISD::SETLE:
  case ISD::SETGT:
  case ISD::SETGE:
  case ISD::SETULT:
  case ISD::SETULE:
  case ISD::SETUGT:
  case ISD::SETUGE: {
    // Useful variables
    // See: llvm/CodeGen/ISDOpcodes.h:1428
    SDValue new_chain = Chain;
    bool is_unsigned = (CC & (1 << 3)) != 0;
    bool is_signed = (CC & (1 << 3)) == 0;
    // Compute nzp
    // See: llvm/CodeGen/ISDOpcodes.h:1428
    uint8_t nzp = 0b000;
    {
      uint8_t n = (CC & (1 << 2)) != 0 ? 0b100 : 0b000;
      uint8_t z = (CC & (1 << 0)) != 0 ? 0b010 : 0b000;
      uint8_t p = (CC & (1 << 1)) != 0 ? 0b001 : 0b000;
      nzp = n | z | p;
    }

    // Compute the value
    SDValue new_value;
    if ((is_unsigned && !UseUnsignedCMPLibCall.getValue()) ||
        (is_signed && !UseSignedCMPLibCall.getValue())) {
      // It's just subtraction if we're not worried about overflow
      new_value = DAG.getNode(ISD::SUB, dl, MVT::i32, LHS, RHS);

    } else {
      // Compute the name of the function to call
      const char *callee_name;
      if (is_signed)
        callee_name = SignedCMPLibCallName.getValue().c_str();
      if (is_unsigned)
        callee_name = UnsignedCMPLibCallName.getValue().c_str();
      // Get the symbol associated with that name
      SDValue callee = DAG.getExternalSymbol(
          callee_name, this->getPointerTy(DAG.getDataLayout()));

      // Calculate the arguments to the libcall
      ArgListTy args;
      args.reserve(2);
      {
        ArgListEntry arg0;
        ArgListEntry arg1;
        arg0.Node = LHS;
        arg1.Node = RHS;
        arg0.Ty = arg1.Ty = EVT(MVT::i32).getTypeForEVT(*DAG.getContext());
        arg0.IsSExt = arg1.IsSExt = true;
        args.push_back(arg0);
        args.push_back(arg1);
      }

      // Calculate the libcall options
      CallLoweringInfo CLI(DAG);
      Type *ret_ty = EVT(MVT::i32).getTypeForEVT(*DAG.getContext());
      CLI.setDebugLoc(dl)
          .setChain(Chain)
          .setCallee(CallingConv::C, ret_ty, callee, std::move(args))
          .setSExtResult(true);

      // Do the call
      std::tie(new_value, new_chain) = this->LowerCallTo(CLI);
    }

    // Return
    return DoCMPResult{
        new_chain,
        DAG.getTargetConstant(nzp, dl, MVT::i32),
        new_value,
    };
  }

  default:
    llvm_unreachable("Bad CC");
  }
}
