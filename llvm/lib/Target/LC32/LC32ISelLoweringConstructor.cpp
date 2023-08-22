//===-- LC32ISelLoweringConstructor.cpp - LC-3.2 DAG Lowering ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32CLOpts.h"
#include "LC32ISelLowering.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
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
  this->setPrefFunctionAlignment(Align(2));

  // Set stack alignment
  this->setMinStackArgumentAlignment(Align(4));

  // Setup booleans
  this->setBooleanContents(ZeroOrOneBooleanContent);
  this->setBooleanVectorContents(ZeroOrOneBooleanContent);

  // Setup VLA
  this->setStackPointerRegisterToSaveRestore(LC32::SP);

  // Optimization options
  this->setJumpIsExpensive();

  // Setup how we should extend loads
  // Not all loads have corresponding instructions
  this->setLoadExtAction(ISD::EXTLOAD, MVT::i32, {MVT::i1, MVT::i4}, Promote);
  this->setLoadExtAction(ISD::SEXTLOAD, MVT::i32, {MVT::i1, MVT::i4}, Promote);
  this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, {MVT::i1, MVT::i4}, Promote);
  this->setLoadExtAction(ISD::ZEXTLOAD, MVT::i32, {MVT::i8, MVT::i16}, Expand);

  // Allow some traps
  // Normally these are expanded to abort()
  this->setOperationAction(ISD::TRAP, MVT::Other, Legal);
  this->setOperationAction(ISD::DEBUGTRAP, MVT::Other, Legal);

  // SUB and OR need custom lowering so we don't go into an infinite loop
  this->setOperationAction(ISD::SUB, MVT::i32, Custom);
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
  this->setOperationAction(ISD::SIGN_EXTEND,
                           {MVT::i1, MVT::i4, MVT::i8, MVT::i16}, Expand);
  this->setOperationAction(ISD::ZERO_EXTEND,
                           {MVT::i1, MVT::i4, MVT::i8, MVT::i16}, Expand);
  this->setOperationAction(ISD::ANY_EXTEND,
                           {MVT::i1, MVT::i4, MVT::i8, MVT::i16}, Expand);
  this->setOperationAction(ISD::TRUNCATE, {MVT::i1, MVT::i4, MVT::i8, MVT::i16},
                           Expand);
  this->setOperationAction(ISD::SIGN_EXTEND_INREG,
                           {MVT::i1, MVT::i4, MVT::i8, MVT::i16}, Expand);

  // Expand stack management instructions
  // This is needed for stack realignment since we don't support it natively
  this->setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
  this->setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  this->setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  // Expand VAArgs wherever possible, otherwise custom
  this->setOperationAction(ISD::VASTART, MVT::Other, Custom);
  this->setOperationAction(ISD::VAARG, MVT::Other, Expand);
  this->setOperationAction(ISD::VAEND, MVT::Other, Expand);
  this->setOperationAction(ISD::VACOPY, MVT::Other, Expand);

  // Expand complex conditionals
  this->setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  this->setOperationAction(ISD::BRCOND, MVT::Other, Expand);
  this->setOperationAction(ISD::SELECT, MVT::i32, Expand);
  this->setOperationAction(ISD::SETCC, MVT::i32, Expand);

  // Custom lowering for base conditionals
  // We also have to handle ISD::BR, but we handle that in TableGen
  this->setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  this->setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);

  // Wrap address operands
  // See td/instr/LC32MemInstrInfo.td
  this->setOperationAction(ISD::GlobalAddress, MVT::i32, Custom);
  this->setOperationAction(ISD::ExternalSymbol, MVT::i32, Custom);
  this->setOperationAction(ISD::ConstantPool, MVT::i32, Custom);
  this->setOperationAction(ISD::JumpTable, MVT::i32, Custom);
  this->setOperationAction(ISD::BlockAddress, MVT::i32, Custom);
  this->setOperationAction(ISD::FrameIndex, MVT::i32, Custom);
}

bool LC32TargetLowering::useSoftFloat() const {
  // No floating point instructions - we have no choice
  return true;
}

bool LC32TargetLowering::isSelectSupported(SelectSupportKind kind) const {
  // Selects are always lowered to branches
  return false;
}

bool LC32TargetLowering::reduceSelectOfFPConstantLoads(EVT CmpOpVT) const {
  // The constant pool isn't fast, so don't bother
  return false;
}

bool LC32TargetLowering::preferZeroCompareBranch() const {
  // Only compares with zero are cheap
  return true;
}

bool LC32TargetLowering::hasBitPreservingFPLogic(EVT VT) const {
  // The standard library uses IEEE-754, so we can convert to bitwise
  return true;
}

bool LC32TargetLowering::convertSetCCLogicToBitwiseLogic(EVT VT) const {
  // SetCC is as expensive as a branch, so avoid it
  return true;
}

bool LC32TargetLowering::hasAndNotCompare(SDValue Y) const {
  // It is profitable to compare with zero in all cases, including this one
  return true;
}

bool LC32TargetLowering::hasAndNot(SDValue X) const {
  // Still, we do not have an and-not instruction
  return false;
}

bool LC32TargetLowering::shouldFoldMaskToVariableShiftPair(SDValue X) const {
  // The two variable shifts variant reduces register pressure
  return true;
}

bool LC32TargetLowering::shouldFoldConstantShiftPairToMask(
    const SDNode *N, CombineLevel Level) const {

  assert(((N->getOpcode() == ISD::SHL &&
           N->getOperand(0).getOpcode() == ISD::SRL) ||
          (N->getOpcode() == ISD::SRL &&
           N->getOperand(0).getOpcode() == ISD::SHL)) &&
         "Expected shift-shift mask");

  // Compute the shift constants
  // If they are not constants, prefer shifts
  SDValue c1 = N->getOperand(0).getOperand(1);
  SDValue c2 = N->getOperand(1);
  if (!isa<ConstantSDNode>(c1) || !isa<ConstantSDNode>(c2))
    return false;
  uint64_t c1_num = dyn_cast<ConstantSDNode>(c1)->getZExtValue();
  uint64_t c2_num = dyn_cast<ConstantSDNode>(c2)->getZExtValue();

  // Compute the mask that results from these shifts
  uint32_t mask = -1;
  switch (N->getOpcode()) {
  case ISD::SHL:
    mask >>= c1_num;
    mask <<= c2_num;
    break;
  case ISD::SRL:
    mask <<= c1_num;
    mask >>= c2_num;
    break;
  default:
    llvm_unreachable("Unhandled case");
  }

  // Check whether the mask can fit
  return isInt<5>(SignExtend64<32>(mask));
}

bool LC32TargetLowering::shouldTransformSignedTruncationCheck(
    EVT XVT, unsigned KeptBits) const {
  // Doing the transformation this controls reduces register pressure
  return true;
}

bool LC32TargetLowering::shouldNormalizeToSelectSequence(LLVMContext &Context,
                                                         EVT VT) const {
  // We can do logic operations on booleans
  return false;
}

bool LC32TargetLowering::convertSelectOfConstantsToMath(EVT VT) const {
  // Selects should be avoided as much as possible
  return true;
}

bool LC32TargetLowering::decomposeMulByConstant(LLVMContext &Context, EVT VT,
                                                SDValue C) const {

  assert(isa<ConstantSDNode>(C) && "Not given a constant");

  // Compute the hamming weight of the constant
  uint32_t c_num = dyn_cast<ConstantSDNode>(C)->getZExtValue();
  unsigned c_weight = 0;
  for (size_t i = 0; i < 32; i++)
    c_weight += (c_num & (1 << i)) ? 1 : 0;

  // Only combine if the hamming weight is small enough
  return c_weight <= MaxConstMulHammingWeight;
}

bool LC32TargetLowering::isMulAddWithConstProfitable(SDValue AddNode,
                                                     SDValue ConstNode) const {
  // Distributing a multiply will usually make constants larger, so it is almost
  // never a good idea to do this
  return false;
}

bool LC32TargetLowering::isLegalICmpImmediate(int64_t Value) const {
  // No way to compare with immediates
  return false;
}

bool LC32TargetLowering::isLegalAddImmediate(int64_t Value) const {
  // Check imm5
  return isInt<5>(Value);
}

bool LC32TargetLowering::isDesirableToCommuteWithShift(
    const SDNode *N, CombineLevel Level) const {

  // Commuting with shift is not desirable when it makes the constant operands
  // bigger - out of range of imm5
  assert((N->getOpcode() == ISD::SHL || N->getOpcode() == ISD::SRA ||
          N->getOpcode() == ISD::SRL) &&
         "Expected shift op");

  // Arithmetic right shift always makes numbers smaller, so that's allowed
  if (N->getOpcode() == ISD::SRA)
    return true;

  // Check that we have the pattern (N (M x c) a) where:
  // * N is SHL or SRL
  // * M is ADD, AND, or XOR
  // * a and c are constants
  // In those cases, we might accidentally make constants larger. If we're not
  // in those cases, we're free to commute.
  SDValue M = N->getOperand(0);
  if (M.getOpcode() != ISD::ADD && M.getOpcode() != ISD::AND &&
      M.getOpcode() != ISD::XOR)
    return true;
  SDValue a = N->getOperand(1);
  SDValue c = M.getOperand(1);
  if (!isa<ConstantSDNode>(a) || !isa<ConstantSDNode>(c))
    return true;
  uint64_t a_num = dyn_cast<ConstantSDNode>(a)->getZExtValue();
  uint64_t c_num = dyn_cast<ConstantSDNode>(c)->getZExtValue();

  // If we're an SHL, check that commuting won't cause us to go out of range for
  // an imm5
  if (N->getOpcode() == ISD::SHL) {
    bool inrange_i = isInt<5>(a_num);
    bool inrange_f = isInt<5>(a_num << c_num);
    return (inrange_i && !inrange_f) ? false : true;
  }
  // If we're an SRL, do the same check
  if (N->getOpcode() == ISD::SRL) {
    bool inrange_i = isInt<5>(a_num);
    bool inrange_f = isInt<5>(a_num >> c_num);
    return (inrange_i && !inrange_f) ? false : true;
  }

  // We handled all the cases, so we should never get here
  llvm_unreachable("Unhandled case");
}

bool LC32TargetLowering::isDesirableToTransformToIntegerOp(unsigned Opc,
                                                           EVT VT) const {
  // We don't have any native floating point support, so it is always welcome if
  // we can get away with integer operations.
  return true;
}
