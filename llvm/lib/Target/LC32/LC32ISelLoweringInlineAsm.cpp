//===-- LC32ISelLoweringInlineAsm.cpp - LC-3.2 DAG Lowering -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32ISelLowering.h"
#include "LC32RegisterInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;
#define DEBUG_TYPE "LC32ISelLoweringInlineAsm"

LC32TargetLowering::ConstraintType
LC32TargetLowering::getConstraintType(StringRef Constraint) const {

  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'r': // Any register
      return C_RegisterClass;
    case 'a': // R0
    case 'x': // R1
    case 'y': // R2
      return C_Register;
    case 'I': // imm5
    case 'N': // amount5
    case 'O': // offset6
      return C_Immediate;
    }
  }

  return TargetLowering::getConstraintType(Constraint);
}

LC32TargetLowering::ConstraintWeight
LC32TargetLowering::getSingleConstraintMatchWeight(
    AsmOperandInfo &info, const char *constraint) const {

  // Everyone checks if the value is null, so we do too
  if (info.CallOperandVal == nullptr)
    return CW_Default;

  if (StringRef(constraint).size() == 1) {
    switch (constraint[0]) {
    default:
      break;
    case 'r': // Any register
      return CW_Register;
    case 'a': // R0
    case 'x': // R1
    case 'y': // R2
      return CW_SpecificReg;
    case 'I': // imm5
    case 'N': // amount5
    case 'O': // offset6
      return CW_Constant;
    }
  }

  return TargetLowering::getSingleConstraintMatchWeight(info, constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
LC32TargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *RI,
                                                 StringRef Constraint,
                                                 MVT VT) const {
  // This function only handles the register-based constraints. Immediate
  // constraints are handled by LowerAsmOperandForConstraint.

  if (Constraint.size() == 1) {
    // All these returns only handle up to i32
    if (VT >= MVT::FIRST_INTEGER_VALUETYPE && VT <= MVT::i32) {
      switch (Constraint[0]) {
      default:
        break;
      case 'r': // Any register
        return std::make_pair(0u, &LC32::GPRRegClass);
      case 'a': // R0
        return std::make_pair((unsigned)LC32::R0, &LC32::GPRRegClass);
      case 'x': // R1
        return std::make_pair((unsigned)LC32::R1, &LC32::GPRRegClass);
      case 'y': // R2
        return std::make_pair((unsigned)LC32::R2, &LC32::GPRRegClass);
      }
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(RI, Constraint, VT);
}

void LC32TargetLowering::LowerAsmOperandForConstraint(SDValue Op,
                                                      std::string &Constraint,
                                                      std::vector<SDValue> &Ops,
                                                      SelectionDAG &DAG) const {
  // This function only handles the immediate-based constraints. Register
  // constraints are handled by getRegForInlineAsmConstraint.
  SDLoc dl(Op);
  EVT ty = Op.getValueType();

  if (Constraint.length() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'I': // imm5
    case 'N': // amount5
    case 'O': // offset6
    {
      // We only handle constants here
      // Try to extract it, and fail if we can't
      const ConstantSDNode *C = dyn_cast<ConstantSDNode>(Op);
      if (C == nullptr)
        return;

      // Get the actual immediate values
      int64_t cs = C->getSExtValue();
      uint64_t cu = C->getZExtValue();

      // Do the tests and append if needed
      switch (Constraint[0]) {
      default:
        llvm_unreachable("Unhandled constraint");
      case 'I':
        if (isInt<5>(cs))
          Ops.push_back(DAG.getTargetConstant(cs, dl, ty));
        return;
      case 'N':
        if (isUInt<5>(cu))
          Ops.push_back(DAG.getTargetConstant(cu, dl, ty));
        return;
      case 'O':
        if (isInt<6>(cs))
          Ops.push_back(DAG.getTargetConstant(cs, dl, ty));
        return;
      }

      llvm_unreachable("Unhandled constraint");
    }
    }
  }

  return TargetLowering::LowerAsmOperandForConstraint(Op, Constraint, Ops, DAG);
}

Register
LC32TargetLowering::getRegisterByName(const char *RegName, LLT Ty,
                                      const MachineFunction &MF) const {
  // Not entirely sure where this is called. This is used for the
  // llvm.read_register and llvm.write_register intrinsics, but I can't generate
  // them manually.
  Register r = StringSwitch<Register>(StringRef(RegName).upper())
                   .Case("R0", LC32::R0)
                   .Case("R1", LC32::R1)
                   .Case("R2", LC32::R2)
                   .Case("R3", LC32::AT)
                   .Case("R4", LC32::GP)
                   .Case("R5", LC32::FP)
                   .Case("R6", LC32::SP)
                   .Case("R7", LC32::LR)
                   .Case("AR", LC32::R0)
                   .Case("XR", LC32::R1)
                   .Case("YR", LC32::R2)
                   .Case("AT", LC32::AT)
                   .Case("GP", LC32::GP)
                   .Case("FP", LC32::FP)
                   .Case("SP", LC32::SP)
                   .Case("LR", LC32::LR)
                   .Default(LC32::NoRegister);

  if (r == LC32::NoRegister)
    report_fatal_error(
        Twine("Invalid register name \"" + StringRef(RegName) + "\"."));
  else
    return r;
}
