//===-- LC32InstrInfo.h - LC-3.2 Instruction Information --------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This module contains information about the registers for the LC-3.2, and
// specifically how they relate to code generation. It also contains the
// `eliminateFrameIndex` method.
//
// Another big responsibility of this module is analyzing branches. This allows
// LLVM to reorder basic blocks. This module is also responsible for branch
// relaxation. For compartmentalization, all this functionality is kept in
// `LC32InstrInfoBranches.cpp`. The rest is in `LC32InstrInfoImpl.cpp`.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H
#define LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H

#include "LC32RegisterInfo.h"
#include "llvm/CodeGen/TargetInstrInfo.h"

#define GET_INSTRINFO_HEADER
#include "LC32GenInstrInfo.inc"

namespace llvm {
class LC32Subtarget;

class LC32InstrInfo : public LC32GenInstrInfo {
public:
  explicit LC32InstrInfo(LC32Subtarget &STI);
  const LC32RegisterInfo &getRegisterInfo() const;
  unsigned getInstSizeInBytes(const MachineInstr &MI) const override;

  void storeRegToStackSlot(MachineBasicBlock &MBB,
                           MachineBasicBlock::iterator MI, Register SrcReg,
                           bool isKill, int FrameIndex,
                           const TargetRegisterClass *RC,
                           const TargetRegisterInfo *TRI,
                           Register VReg) const override;
  void loadRegFromStackSlot(MachineBasicBlock &MBB,
                            MachineBasicBlock::iterator MI, Register DestReg,
                            int FrameIndex, const TargetRegisterClass *RC,
                            const TargetRegisterInfo *TRI,
                            Register VReg) const override;

  void copyPhysReg(MachineBasicBlock &MBB, MachineBasicBlock::iterator MI,
                   const DebugLoc &DL, MCRegister DestReg, MCRegister SrcReg,
                   bool KillSrc) const override;

  bool analyzeBranch(MachineBasicBlock &MBB, MachineBasicBlock *&TBB,
                     MachineBasicBlock *&FBB,
                     SmallVectorImpl<MachineOperand> &Cond,
                     bool AllowModify = false) const override;
  unsigned removeBranch(MachineBasicBlock &MBB,
                        int *BytesRemoved = nullptr) const override;
  unsigned insertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                        MachineBasicBlock *FBB, ArrayRef<MachineOperand> Cond,
                        const DebugLoc &DL,
                        int *BytesAdded = nullptr) const override;
  bool
  reverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const override;

  bool isBranchOffsetInRange(unsigned BranchOpc,
                             int64_t BrOffset) const override;
  MachineBasicBlock *getBranchDestBlock(const MachineInstr &MI) const override;
  void insertIndirectBranch(MachineBasicBlock &MBB,
                            MachineBasicBlock &NewDestBB,
                            MachineBasicBlock &RestoreBB, const DebugLoc &DL,
                            int64_t BrOffset = 0,
                            RegScavenger *RS = nullptr) const override;

private:
  // Modules need the register information to work with this class. Therefore,
  // we provide access to it.
  // See: MSP430InstrInfo.h
  LC32RegisterInfo RegisterInfo;
};

} // namespace llvm

#endif // LLVM_LIB_TARGET_LC32_LC32INSTRINFO_H
