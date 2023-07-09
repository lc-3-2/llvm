//==- LC32FrameLowering.cpp - Define Frame Lowering for LC-3.2 --*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32FrameLowering.h"
#include "LC32CLOpts.h"
#include "LC32InstrInfo.h"
#include "LC32MachineFunctionInfo.h"
#include "LC32Subtarget.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32FrameLowering"

LC32FrameLowering::LC32FrameLowering()
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, Align(4), -12,
                          Align(4), false) {}

bool LC32FrameLowering::hasFP(const MachineFunction &MF) const { return true; }

void LC32FrameLowering::emitPrologue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {
  // Populate variables
  // If the first block is empty, the debug location will be invalid. Deal with
  // that
  assert(&MF.front() == &MBB && "Shrink-wrapping not supported");
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

  // This is relative from the end of the fixed variables section and in the
  // "wrong" direction. It is used when computing frame offsets via
  // getFrameIndexReference.
  MFI.setOffsetAdjustment(-static_cast<int64_t>(MFI.getStackSize()) +
                          INT64_C(4));

  // Save LR and FP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::C_STLR)).addReg(LC32::SP).addImm(-8);
  BuildMI(MBB, MBBI, dl, TII.get(LC32::STW))
      .addReg(LC32::FP, RegState::Kill)
      .addReg(LC32::SP)
      .addImm(-12);

  // Compute FP
  // Remember, we always have one word for local variables
  BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::FP)
      .addReg(LC32::SP, RegState::Kill)
      .addImm(-16);

  // Build the stack pointer
  // Remember, we always have one word for local variables
  TRI.genAddLargeImm(
      MBBI, dl, LC32::SP, LC32::FP,
      std::min(-static_cast<int64_t>(MFI.getStackSize()) + INT64_C(4),
               INT64_C(0)));
}

void LC32FrameLowering::emitEpilogue(MachineFunction &MF,
                                     MachineBasicBlock &MBB) const {

  // Populate variables
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  DebugLoc dl = MBBI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Restore SP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::ADDi), LC32::SP)
      .addReg(LC32::FP, RegState::Kill)
      .addImm(12);

  // Restore LR and FP
  BuildMI(MBB, MBBI, dl, TII.get(LC32::C_LDLR)).addReg(LC32::SP).addImm(-4);
  BuildMI(MBB, MBBI, dl, TII.get(LC32::LDW), LC32::FP)
      .addReg(LC32::SP)
      .addImm(-8);
}

MachineBasicBlock::iterator LC32FrameLowering::eliminateCallFramePseudoInstr(
    MachineFunction &MF, MachineBasicBlock &MBB,
    MachineBasicBlock::iterator MI) const {
  // Populate variables
  DebugLoc dl = MI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());
  const LC32RegisterInfo &TRI = TII.getRegisterInfo();

  // Check the type of the variable we're eliminating
  assert(TII.getCallFrameSetupOpcode() != TII.getCallFrameDestroyOpcode() &&
         "Call frame setup and destroy are same");
  assert((MI->getOpcode() == TII.getCallFrameSetupOpcode() ||
          MI->getOpcode() == TII.getCallFrameDestroyOpcode()) &&
         "Bad type of instruction to eliminate");

  // Get the second parameter to this instruction
  // For call frame setups, this is the amount that was set up prior to the
  // chain, which should always be zero for us. For teardowns, this could be
  // zero or four depending on whether we made a call.
  uint64_t internal_amt = MI->getOperand(1).getImm();
  if (MI->getOpcode() == TII.getCallFrameSetupOpcode())
    assert(internal_amt == 0 &&
           "Internal amount should be zero for frame setup");
  if (MI->getOpcode() == TII.getCallFrameDestroyOpcode())
    assert((internal_amt == 0 || internal_amt == 4) &&
           "Internal amount should be zero or four for frame destroy");

  // Get the size associated with this pseudo
  uint64_t amt = TII.getFrameSize(*MI) + internal_amt;
  // Align to four bytes
  assert(this->getStackAlign() == 4 && "LC-3.2 stack should be word aligned");
  amt = alignTo(amt, this->getStackAlign());

  // If the amount is zero, we don't have to do anything
  // The amt above should include the return value if we need to pop it
  if (amt != 0) {
    // Handle setup and teardown
    if (MI->getOpcode() == TII.getCallFrameSetupOpcode()) {
      TRI.genAddLargeImm(MI, dl, LC32::SP, LC32::SP, -amt, true,
                         RegState::Define, RegState::Kill);
    } else if (MI->getOpcode() == TII.getCallFrameDestroyOpcode()) {
      TRI.genAddLargeImm(MI, dl, LC32::SP, LC32::SP, amt, true,
                         RegState::Define, RegState::Kill);
    } else {
      llvm_unreachable("Tried to eliminate bad instruction");
    }
  }

  // Remember to erase the original pseudo
  return MBB.erase(MI);
}

static unsigned EstimateFunctionSize(const MachineFunction &MF,
                                     const LC32InstrInfo &TII) {
  // Used by processFunctionBeforeFrameFinalized
  // See: RISCVFrameLowering.cpp
  unsigned ret = 0;
  for (auto &MBB : MF) {
    for (auto &MI : MBB) {
      // We shouldn't have raw branches at this point
      assert(MI.getOpcode() != LC32::BR && "BR generated");

      // We may still have the C_LEA_FRAMEINDEX instruction. Handle it
      // conservatively, looking at how many bytes it could possibly produce.
      // See: LC32RegisterInfo::eliminateFrameIndex
      if (MI.getOpcode() == LC32::C_LEA_FRAMEINDEX) {
        ret += std::max(2u * MaxRepeatedOps.getValue(), 14u + 2u);
        continue;
      }

      // We don't want to use the expanded versions of branches, even if we're
      // going to do branch relaxation. That's because this function is trying
      // to figure out if we're going to have to expand branches in the first
      // place. If the function is small enough, we don't have to expand. All
      // that is to say, we should use getInstSizeInBytes raw.
      ret += TII.getInstSizeInBytes(MI);
    }
  }
  // We should only ever return even values since all instructions are aligned
  // to two bytes
  assert(ret % 2 == 0 && "Functions should have even length");
  return ret;
}

void LC32FrameLowering::processFunctionBeforeFrameFinalized(
    MachineFunction &MF, RegScavenger *RS) const {
  // Check that we actually got a register scavenger
  assert(RS != nullptr && "Must have a register scavenger");
  // Populate variables
  const LC32RegisterInfo *TRI =
      MF.getSubtarget<LC32Subtarget>().getRegisterInfo();
  const LC32InstrInfo *TII = MF.getSubtarget<LC32Subtarget>().getInstrInfo();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  LC32MachineFunctionInfo *MFnI = MF.getInfo<LC32MachineFunctionInfo>();

  // Count the number of scavenging slots we need:
  // A. if the stack frame is too large. This is a slight underestimate, so
  //    compensate
  // B. if branches can be out of range. This is an overestimate, so we
  //    shouldn't need to compensate here
  // Note that these two conditions are independent. Clearly, we will never have
  // a branch that happens at the same time as a stack frame load. Therefore,
  // these can share scavenging slots.
  //
  // Also note that in both the checks, we keep the arguments positive since
  // that is the more restrictive direction. Observe that we use isInt and not
  // isUInt.
  unsigned num_scav = 0;

  // Do A.
  // This is restricted by STB, which has a 6-bit immediate with no shift. Only
  // the stores use this frame index
  // This is also restricted by C_LEA_FRAMEINDEX, which requires us to check
  // MaxRepeatedOps.
  bool need_a = !isInt<6 - 1>(MFI.estimateStackSize(MF));
  if (MaxRepeatedOps.getValue() == 0)
    need_a |= true;
  if (MaxRepeatedOps.getValue() == 1)
    need_a |= !isInt<5 - 1>(MFI.estimateStackSize(MF));
  if (need_a)
    num_scav = std::max(1u, num_scav);
  // Do B. IF we do branch relaxation
  // BR has a 9-bit immediate, but it's doubled in hardware. Note that
  // EstimateFunctionSize will always return an even value.
  bool need_b = !isShiftedInt<9, 1>(EstimateFunctionSize(MF, *TII));
  if (need_b)
    num_scav = std::max(1u, num_scav);

  // Create the scavenging indicies
  for (unsigned i = 0; i < num_scav; i++) {
    int FI =
        MFI.CreateStackObject(TRI->getSpillSize(LC32::GPRRegClass),
                              TRI->getSpillAlign(LC32::GPRRegClass), false);
    RS->addScavengingFrameIndex(FI);

    // The first scavening frame index we use should be used for branch
    // relaxation. Note that this should happen on the first iteration of the
    // loop if it happens at all
    assert(MFnI->BranchRelaxationFI == -1);
    assert(FI >= 0);
    if (need_b && MFnI->BranchRelaxationFI == -1)
      MFnI->BranchRelaxationFI = FI;
  }
}
