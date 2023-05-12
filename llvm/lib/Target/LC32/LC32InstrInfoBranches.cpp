//===-- LC32InstrInfoBranches.h - LC-3.2 Instruction Info -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
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
  // We know that the block ends with BR and C_BR_CMP_ZERO
  MachineBasicBlock::iterator i = MBB.end();
  while (i != MBB.begin()) {
    assert(i == MBB.end());
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
