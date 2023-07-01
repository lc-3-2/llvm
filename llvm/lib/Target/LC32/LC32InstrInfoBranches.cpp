//===-- LC32InstrInfoBranches.h - LC-3.2 Instruction Info -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "LC32MachineFunctionInfo.h"
#include "LC32RegisterInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/CodeGen/RegisterScavenging.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;
#define DEBUG_TYPE "LC32InstrInfoBranches"

bool LC32InstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                  MachineBasicBlock *&TBB,
                                  MachineBasicBlock *&FBB,
                                  SmallVectorImpl<MachineOperand> &Cond,
                                  bool AllowModify) const {
  // See: llvm/CodeGen/TargetInstrInfo.h:641
  // See: MSP430InstrInfo.cpp:164

  // Start from the bottom of the block
  MachineBasicBlock::iterator i = MBB.end();
  while (i != MBB.begin()) {
    i--;

    // Ignore debug instructions
    if (i->isDebugInstr())
      continue;

    // If we hit a non-terminator, we've reached the end of the bit that's
    // interesting for us. So, optimize by bailing
    if (!i->isTerminator())
      break;

    // Can't handle non-branches
    // Like: C_RET
    if (!i->isBranch())
      return true;
    // Can't handle indirect branches
    // Like: JMP
    if (i->isIndirectBranch())
      return true;

    // Check the instruction has an expected opcode
    // BR and FARBR shouldn't exist at this point
    assert(i->getOpcode() != LC32::BR && "BR generated");
    assert(i->getOpcode() != LC32::P_FARBR && "FARBR generated");
    assert((i->getOpcode() == LC32::C_BR_UNCOND ||
            i->getOpcode() == LC32::C_BR_CMP_ZERO) &&
           "Bad terminator");

    // Handle unconditional branches. This sets TBB to the branch destination
    // and continues. If the preceeding instruction is conditional, it can refer
    // to TBB.
    if (i->getOpcode() == LC32::C_BR_UNCOND) {

      // Set TBB as needed
      TBB = i->getOperand(0).getMBB();
      FBB = nullptr;
      // Reset conditions
      // Even though we may have seen conditional branches after this, we can't
      // reach them
      Cond.clear();

      // If we're allowed to modify, delete all the instructions after this. If
      // we're going to the successor, delete this too and handle as fallthrough
      if (AllowModify) {
        MBB.erase(std::next(i), MBB.end());
        if (MBB.isLayoutSuccessor(TBB)) {
          TBB = nullptr;
          i->eraseFromParent();
          i = MBB.end();
        }
      }

      // We may have a conditional branch after this
      continue;
    }

    // Handle conditional branches.
    if (i->getOpcode() == LC32::C_BR_CMP_ZERO) {
      // Get NZP and check its good
      int64_t nzp = i->getOperand(0).getImm();
      assert(nzp >= 0b000 && nzp <= 0b111 && "Bad nzp");
      assert(nzp != 0b000 && "NOP generated");

      // If this isn't our first one, die
      // We can't decode multiple conditional branches at the end
      if (Cond.size() != 0)
        return true;

      // Set data
      // Remember, TBB is either nullptr or was set above
      Cond.push_back(MachineOperand::CreateImm(nzp));
      Cond.push_back(i->getOperand(1));
      FBB = TBB;
      TBB = i->getOperand(2).getMBB();

      // Check we don't have a conditional above this
      continue;
    }

    llvm_unreachable("Unhandled opcode");
  }

  // We may have seen branches - TBB and FBB should be set by now
  return false;
}

unsigned LC32InstrInfo::removeBranch(MachineBasicBlock &MBB,
                                     int *BytesRemoved) const {

  // Keep a count of instructions and the size removed
  unsigned count = 0;
  unsigned bytes = 0;

  // Iterate backward
  // We know that the block ends with BR and C_BR_CMP_ZERO, but there may be
  // debug instructions after it
  MachineBasicBlock::iterator i = MBB.end();
  while (i != MBB.begin()) {
    i--;

    // Ignore the same set of instructions analyzeBranch does
    if (i->isDebugInstr())
      continue;
    if (!i->isTerminator())
      break;

    // Check the opcodes
    assert((i->getOpcode() == LC32::C_BR_UNCOND ||
            i->getOpcode() == LC32::C_BR_CMP_ZERO) &&
           "Bad terminator");

    // Update statistics
    count++;
    bytes += this->getInstSizeInBytes(*i);

    // Remove
    // Make sure to go to the end of the basic block after removal. The iterator
    // is invalid now.
    // FIXME: Performance
    i->eraseFromParent();
    i = MBB.end();
  }

  // Return
  if (BytesRemoved != nullptr)
    *BytesRemoved = bytes;
  return count;
}

unsigned LC32InstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {

  // Keep a count of instructions and the size removed
  unsigned count = 0;
  unsigned bytes = 0;

  // Check conditions are good
  assert((Cond.empty() || Cond.size() == 2) && "Bad size for conditions");

  if (TBB == nullptr) {
    // We were told to insert a fallthrough
    // Do nothing
    assert(FBB == nullptr && "Bad fallthrough form");
    assert(Cond.empty() && "Bad conditions for fallthrough");

  } else if (Cond.empty()) {
    // We should insert an unconditional branch
    assert(FBB == nullptr && "Unconditional can't have fallthrough");

    // Create the instruction
    MachineInstr *mi =
        BuildMI(&MBB, DL, this->get(LC32::C_BR_UNCOND)).addMBB(TBB);
    // Update stats
    count++;
    bytes += this->getInstSizeInBytes(*mi);

  } else if (Cond.size() == 2) {
    // We should insert a conditional branch

    // Create the first branch and update stats
    // Make sure to preserve register state
    MachineInstr *mi = BuildMI(&MBB, DL, this->get(LC32::C_BR_CMP_ZERO))
                           .add(Cond[0])
                           .add(Cond[1])
                           .addMBB(TBB);
    count++;
    bytes += this->getInstSizeInBytes(*mi);

    // Second branch if needed, and update stats
    if (FBB != nullptr) {
      mi = BuildMI(&MBB, DL, this->get(LC32::C_BR_UNCOND)).addMBB(FBB);
      count++;
      bytes += this->getInstSizeInBytes(*mi);
    }

  } else {
    llvm_unreachable("Bad Cond");
  }

  // Return
  if (BytesAdded != nullptr)
    *BytesAdded = bytes;
  return count;
}

bool LC32InstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  // Check preconditions
  assert(Cond.size() == 2 && "Bad conditions for conditional");
  assert(Cond[0].getImm() >= 0b000 && Cond[0].getImm() <= 0b111 && "Bad nzp");
  assert(Cond[0].getImm() != 0b000 && Cond[0].getImm() != 0b111 && "Bad nzp");
  // Return
  Cond[0].setImm(~Cond[0].getImm() & 0b111);
  return false;
}

bool LC32InstrInfo::isBranchOffsetInRange(unsigned BranchOpc,
                                          int64_t BrOffset) const {
  // We should only get branch instructions in here, and never indirect
  // branches. Also, we should never have BR or FARBR because they haven't been
  // generated at this point.
  assert(BranchOpc != LC32::BR && "BR generated");
  assert(BranchOpc != LC32::P_FARBR && "FARBR generated");
  assert((BranchOpc == LC32::C_BR_UNCOND || BranchOpc == LC32::C_BR_CMP_ZERO) &&
         "Bad terminator");
  // All instructions have even length, so the branch offset should be even
  assert(BrOffset % 2 == 0 && "Branch offset should be even");

  // All possible instructions use PCOffset9
  return isShiftedInt<9, 1>(BrOffset - 2);
}

MachineBasicBlock *
LC32InstrInfo::getBranchDestBlock(const MachineInstr &MI) const {
  // Whatever we get here should either satisfy isConditionalBranch or
  // isUnconditionalBranch. As always, we should never get BR or FARBR either.
  assert(MI.getOpcode() != LC32::BR && "BR generated");
  assert(MI.getOpcode() != LC32::P_FARBR && "FARBR generated");
  assert((MI.getOpcode() == LC32::C_BR_UNCOND ||
          MI.getOpcode() == LC32::C_BR_CMP_ZERO) &&
         "Bad terminator");
  // Return the branch target for each case
  switch (MI.getOpcode()) {
  case LC32::C_BR_UNCOND:
    return MI.getOperand(0).getMBB();
  case LC32::C_BR_CMP_ZERO:
    return MI.getOperand(2).getMBB();
  default:
    llvm_unreachable("Unhandled case");
  }
}

void LC32InstrInfo::insertIndirectBranch(MachineBasicBlock &MBB,
                                         MachineBasicBlock &NewDestBB,
                                         MachineBasicBlock &RestoreBB,
                                         const DebugLoc &DL, int64_t BrOffset,
                                         RegScavenger *RS) const {
  // This code is mostly copied from other backends. The RISCV backend was
  // particularly helpful, as was the LoongArch Backend

  // Preconditions. These are guaranteed by the pass, but the other backends
  // still check them.
  assert(RS != nullptr &&
         "RegScavenger required for spilling registers for long branching");
  assert(MBB.empty() &&
         "New block should be inserted for expanding unconditional branch");
  assert(MBB.pred_size() == 1 &&
         "New block should only have one predecessor: the original block");
  assert(RestoreBB.empty() &&
         "Restore block should be inserted for restoring clobbered registers");

  // Useful variables
  MachineFunction *MF = MBB.getParent();
  MachineRegisterInfo &MRI = MF->getRegInfo();
  LC32MachineFunctionInfo *MFnI = MF->getInfo<LC32MachineFunctionInfo>();
  const TargetRegisterInfo *TRI = MF->getSubtarget().getRegisterInfo();

  // Check that we actually populated the branch relaxation frame index
  // See: LC32FrameLowering::processFunctionBeforeFrameFinalized
  assert(MFnI->BranchRelaxationFI != -1 &&
         "Didn't populate branch relaxation frame index");

  // We want to use register scavenging to figure out what free registers we can
  // use as a scratchpad. However, it currently doesn't work with empty blocks.
  // Therefore, we construct instructions with a virtual scratch register and
  // replace it later.
  Register vreg = MRI.createVirtualRegister(&LC32::GPRRegClass);

  // Construct instructions at the end of MBB that do the indirect branch. We
  // set the destination to NewDestBB for now, but we may set it to RestoreBB if
  // we need to spill a register and do cleanup.
  MachineInstr &ldc =
      *BuildMI(MBB, MBB.end(), DL, this->get(LC32::P_LOADCONSTW), vreg)
           .addMBB(&NewDestBB);
  MachineInstr &jmp = *BuildMI(MBB, MBB.end(), DL, this->get(LC32::JMP))
                           .addReg(vreg, RegState::Kill);
  // We only need references to things that reference the destination bb
  (void)jmp;

  // Try to scavenge a physical register to replace the vierual register. Don't
  // spill or restore registers during this. We will do that ourselves since it
  // needs to go in the restore block
  RS->enterBasicBlockEnd(MBB);
  Register preg = RS->scavengeRegisterBackwards(
      LC32::GPRRegClass, ldc.getIterator(), false, 0, false);

  // Check the scavenging worked
  if (preg != LC32::NoRegister) {
    RS->setRegUsed(preg);

  } else {
    // We were unable to scavenge an unused register, so use AT
    preg = LC32::AT;

    // Spill AT before we use it as a scratch register
    this->storeRegToStackSlot(MBB, ldc.getIterator(), LC32::AT, true,
                              MFnI->BranchRelaxationFI, &LC32::GPRRegClass, TRI,
                              Register());
    TRI->eliminateFrameIndex(std::prev(ldc.getIterator()), 0, 1);

    // Restore the AT in the restore block
    this->loadRegFromStackSlot(RestoreBB, RestoreBB.end(), LC32::AT,
                               MFnI->BranchRelaxationFI, &LC32::GPRRegClass,
                               TRI, Register());
    TRI->eliminateFrameIndex(RestoreBB.back(), 0, 1);

    // Update our branch to point to the restore block, which falls through to
    // the destination block
    ldc.getOperand(1).setMBB(&RestoreBB);
  }

  // Substitute the virtual register for the physical register
  MRI.replaceRegWith(vreg, preg);
  MRI.clearVirtRegs();
}
