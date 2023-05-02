//===-- LC32ISelLoweringOps.cpp - LC-3.2 DAG Lowering Interface -*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLoweringOps"

const char *LC32TargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  case LC32ISD::OR_LOWERING_NOT:
    return "LC32ISD::OR_LOWERING_NOT";
  case LC32ISD::CALL:
    return "LC32ISD::CALL";
  case LC32ISD::RET:
    return "LC32ISD::RET";
  case LC32ISD::CMP_ZERO:
    return "LC32ISD::CMP_ZERO";
  case LC32ISD::BR:
    return "LC32ISD::BR";
  default:
    return nullptr;
  }
}

SDValue LC32TargetLowering::LowerOperation(SDValue Op,
                                           SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
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
  case LC32ISD::OR_LOWERING_NOT:
  case LC32ISD::CALL:
  case LC32ISD::RET:
  case LC32ISD::CMP_ZERO:
  case LC32ISD::BR:
    return SDValue();
  default:
    llvm_unreachable("Bad opcode for custom combine");
  }
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
  return DAG.getNode(LC32ISD::OR_LOWERING_NOT, dl, MVT::i32, x_prime);
}

SDValue LC32TargetLowering::visitXOR(SDNode *N, DAGCombinerInfo &DCI) const {
  // fold (not (N_OR_LOWERING_NOT x)) -> x
  if (N->getOperand(0).getNode() != nullptr &&
      N->getOperand(0).getOpcode() == LC32ISD::OR_LOWERING_NOT &&
      isAllOnesConstant(N->getOperand(1))) {
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

  // Check that comparisons have the right type
  assert(LHS.getValueType() == MVT::i32 &&
         "Only i32 is supported for comparison");
  assert(RHS.getValueType() == MVT::i32 &&
         "Only i32 is supported for comparison");

  // Compute the NZP to use
  // Also generate the node to set the condition codes
  uint8_t nzp;
  SDValue cmp_res;
  if (CC == ISD::SETEQ || CC == ISD::SETNE) {
    // Comparison should be an XOR
    SDValue xor_res = DAG.getNode(ISD::XOR, dl, MVT::i32, LHS, RHS);
    cmp_res = DAG.getNode(LC32ISD::CMP_ZERO, dl, MVT::Glue, xor_res);
    // Set nzp
    switch (CC) {
    case ISD::SETEQ:
      nzp = 0b010;
      break;
    case ISD::SETNE:
      nzp = 0b101;
      break;
    default:
      llvm_unreachable("Bad CC");
    }

  } else if (CC == ISD::SETLT || CC == ISD::SETLE || CC == ISD::SETGT ||
             CC == ISD::SETGE || CC == ISD::SETULT || CC == ISD::SETULE ||
             CC == ISD::SETUGT || CC == ISD::SETUGE) {
    // Comparison is done via subtraction
    SDValue sub_res = DAG.getNode(ISD::SUB, dl, MVT::i32, LHS, RHS);
    cmp_res = DAG.getNode(LC32ISD::CMP_ZERO, dl, MVT::Glue, sub_res);
    // Set nzp
    switch (CC) {
    case ISD::SETLT:
    case ISD::SETULT:
      nzp = 0b100;
      break;
    case ISD::SETLE:
    case ISD::SETULE:
      nzp = 0b110;
      break;
    case ISD::SETGT:
    case ISD::SETUGT:
      nzp = 0b001;
      break;
    case ISD::SETGE:
    case ISD::SETUGE:
      nzp = 0b011;
      break;
    default:
      llvm_unreachable("Bad CC");
    }

  } else {
    llvm_unreachable("Unimplemented CC");
  }

  // Emit the branch instruction
  return DAG.getNode(LC32ISD::BR, dl, Op.getValueType(), Chain,
                     DAG.getTargetConstant(nzp, dl, MVT::i32), Target, cmp_res);
}

SDValue LC32TargetLowering::LowerSELECT_CC(SDValue Op,
                                           SelectionDAG &DAG) const {
  // Populate variables
  SDLoc dl(Op);

  llvm_unreachable("Unimplemented");
}
