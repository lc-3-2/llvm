//===-- LC32ISelLoweringConstructor.cpp - LC-3.2 DAG Lowering ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLoweringConstructor"

LC32TargetLowering::LC32TargetLowering(const TargetMachine &TM,
                                       const LC32Subtarget &STI)
    : TargetLowering(TM) {

  // Setup register classes
  this->TRI = STI.getRegisterInfo();
  this->addRegisterClass(MVT::i32, &LC32::GPRRegClass);
  this->computeRegisterProperties(this->TRI);

  // Follow source order when scheduling
  this->setSchedulingPreference(Sched::Source);

  // Set function alignment
  this->setMinFunctionAlignment(Align(2));
  this->setPrefFunctionAlignment(Align(4));

  // Set stack alignment
  this->setMinStackArgumentAlignment(Align(4));

  // Setup booleans
  this->setBooleanContents(ZeroOrOneBooleanContent);
  this->setBooleanVectorContents(ZeroOrOneBooleanContent);

  // Setup VLA
  this->setStackPointerRegisterToSaveRestore(LC32::SP);

  // Setup how we should extend loads
  // Not all loads have corresponding instructions
  for (const auto &vt : {MVT::i1, MVT::i4}) {
    this->setLoadExtAction(ISD::EXTLOAD, MVT::i32, vt, Promote);
    this->setLoadExtAction(ISD::SEXTLOAD, MVT::i32, vt, Promote);
    this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, vt, Promote);
  }
  for (const auto &vt : {MVT::i8, MVT::i16}) {
    this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, vt, Expand);
  }

  // Subtract can be expanded by LLVM
  this->setOperationAction(ISD::SUB, MVT::i32, Expand);
  // LLVM doesn't know to expand OR via DeMorgan's, so we have to do it
  // ourselves. We also give DAG combining information.
  this->setOperationAction(ISD::OR, MVT::i32, Custom);
  this->setTargetDAGCombine(ISD::XOR);

  // Do libcalls for multiplications and divisions
  // Expand operations that are based on them
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

  // Expand complex bitwise
  this->setOperationAction(ISD::ROTL, MVT::i32, Expand);
  this->setOperationAction(ISD::ROTR, MVT::i32, Expand);
  this->setOperationAction(ISD::SHL_PARTS, MVT::i32, Expand);
  this->setOperationAction(ISD::SRA_PARTS, MVT::i32, Expand);
  this->setOperationAction(ISD::SRL_PARTS, MVT::i32, Expand);
  this->setOperationAction(ISD::BSWAP, MVT::i32, Expand);
  this->setOperationAction(ISD::CTTZ, MVT::i32, Expand);
  this->setOperationAction(ISD::CTLZ, MVT::i32, Expand);
  this->setOperationAction(ISD::CTPOP, MVT::i32, Expand);
  this->setOperationAction(ISD::BITREVERSE, MVT::i32, Expand);
  this->setOperationAction(ISD::PARITY, MVT::i32, Expand);
  this->setOperationAction(ISD::CTTZ_ZERO_UNDEF, MVT::i32, Expand);
  this->setOperationAction(ISD::CTLZ_ZERO_UNDEF, MVT::i32, Expand);

  // Extension doesn't have custom instructions
  for (const auto &vt : {MVT::i1, MVT::i4, MVT::i8, MVT::i16}) {
    this->setOperationAction(ISD::SIGN_EXTEND, vt, Expand);
    this->setOperationAction(ISD::ZERO_EXTEND, vt, Expand);
    this->setOperationAction(ISD::ANY_EXTEND, vt, Expand);
    this->setOperationAction(ISD::TRUNCATE, vt, Expand);
    this->setOperationAction(ISD::SIGN_EXTEND_INREG, vt, Expand);
  }

  // Expand complex conditionals
  this->setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  this->setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  this->setOperationAction(ISD::SELECT, MVT::i32, Expand);
  this->setOperationAction(ISD::SETCC, MVT::i32, Expand);

  // Custom lowering for base conditionals
  // We also have to handle ISD::BR, but we handle that in TableGen
  this->setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  this->setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
}
