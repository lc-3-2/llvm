//==- LC32RedundantClearElimination.cpp - Remove Register Clears *- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "pass/LC32RedundantClearElimination.h"
#include "LC32CLOpts.h"
#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32InstPrinter.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32RedundantClearElimination"

char LC32RedundantClearElimination::ID = 0;

INITIALIZE_PASS(LC32RedundantClearElimination, DEBUG_TYPE,
                "LC-3.2 Redundant Register Clear Elimination", false, false)

STATISTIC(NumClearsEliminated,
          "Number of redundant register clears eliminated");

bool LC32RedundantClearElimination::runOnMachineFunction(MachineFunction &MF) {

  // Bail if we were told not to execute this pass
  if (this->skipFunction(MF.getFunction()))
    return false;
  if (!EnableRedundantClearElimination)
    return false;

  // Print standard header
  LLVM_DEBUG(dbgs() << "\n********** " << this->getPassName() << " ********\n");

  // Run on every basic block
  bool MadeChange = false;
  for (MachineBasicBlock &MBB : MF)
    MadeChange |= this->runOnMachineBasicBlock(MBB);

  // Done
  return MadeChange;
}

bool LC32RedundantClearElimination::runOnMachineBasicBlock(
    MachineBasicBlock &MBB) {
  LLVM_DEBUG(dbgs() << "Running on MBB " << MBB.getNumber() << '\n');

  // Populate variables
  MachineFunction &MF = *MBB.getParent();
  const TargetRegisterInfo &TRI = *MF.getSubtarget().getRegisterInfo();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // In order for this pass to work, this block has to have a unique
  // predecessor. This way, we can check whether a register is zero just by
  // looking at the conditional branch at the end of `PredMBB`.
  if (MBB.pred_size() != 1)
    return false;
  MachineBasicBlock *PredMBB = *MBB.pred_begin();

  // The predecessor has to end in a conditional branch for this pass to be
  // relevant, and for us to parse the conditional it has to have exactly two
  // successors
  if (PredMBB->succ_size() != 2)
    return false;

  // Try to analyze the branch. This is the branch in the predecessor, not the
  // current
  MachineBasicBlock *TBB = nullptr;
  MachineBasicBlock *FBB = nullptr;
  SmallVector<MachineOperand, 2> Cond;
  // If we failed to understand it, bail
  if (TII.analyzeBranch(*PredMBB, TBB, FBB, Cond, false))
    return false;
  // If we ended in an unconditional branch, bail
  if (Cond.empty())
    return false;

  // Check whether this is a branch against zero
  if (!this->guaranteesZeroRegInBlock(MBB, Cond, TBB))
    return false;

  // Pull out the register the branch was on
  Register TargetReg = Cond[1].getReg();
  assert(TargetReg != LC32::NoRegister &&
         "Conditional branches should have a register");
  LLVM_DEBUG(dbgs() << "Working with TargetReg = "
                    << LC32InstPrinter::getRegisterName(TargetReg) << '\n');

  // Remove redundant clear instructions. Break if we use the register for
  // something else. Also keep track of whether we changed anything and what the
  // last instruction we changed was. We need that information to handle
  // liveliness information.
  bool Changed = false;
  MachineBasicBlock::iterator LastChange = MBB.begin();
  for (MachineBasicBlock::iterator MBBI = MBB.begin(); MBBI != MBB.end();) {

    // Pull out the machine instruction for convenience
    // This way, we're fine if we delete the instruction
    MachineInstr &MI = *MBBI;
    MBBI++;

    // Check for clear
    LLVM_DEBUG(dbgs() << "Checking instruction: ");
    LLVM_DEBUG(MI.print(dbgs()));
    if (MI.getOpcode() == LC32::C_LOADZERO &&
        MI.getOperand(0).getReg() == TargetReg) {
      // Log
      LLVM_DEBUG(dbgs() << "Remove redundant register clear: ");
      LLVM_DEBUG(MI.print(dbgs()));
      NumClearsEliminated++;
      // Erase
      MI.eraseFromParent();
      // Update state. Note that LastChange points *just past* the last change,
      // not directly to it
      Changed = true;
      LastChange = MBBI;
      // Done with this iteration
      continue;
    }

    // If we start using the register for something else, break
    if (MI.modifiesRegister(TargetReg)) {
      LLVM_DEBUG(dbgs() << "Breaking");
      break;
    }
  }

  // Didn't change anything, so don't have to fix liveliness
  if (!Changed)
    return false;

  // Pull out the conditional branch we just analyzed. Mark its register as not
  // killed.
  MachineBasicBlock::iterator ConditionalBranch = PredMBB->getFirstTerminator();
  assert(ConditionalBranch->getOpcode() == LC32::C_BR_CMP_ZERO &&
         "Unexpected opcode");
  assert(ConditionalBranch->getOperand(1).getReg() == TargetReg &&
         "Unexpected register");
  ConditionalBranch->clearRegisterKills(TargetReg, &TRI);

  // Now this register is a live-in, so add it
  if (!MBB.isLiveIn(TargetReg))
    MBB.addLiveIn(TargetReg);

  // Clear any kills in the target basic block, up to where we last changed
  for (MachineBasicBlock::iterator MBBI = MBB.begin(); MBBI != LastChange;
       MBBI++)
    MBBI->clearRegisterKills(TargetReg, &TRI);

  return true;
}

bool LC32RedundantClearElimination::guaranteesZeroRegInBlock(
    MachineBasicBlock &MBB, const SmallVectorImpl<MachineOperand> &Cond,
    MachineBasicBlock *TBB) {
  // Initial validation
  assert(Cond.size() == 2 && "Unexpected number of operands");
  assert(TBB != nullptr && "Expected branch target basic block");
  // Validate nzp
  int64_t nzp = Cond[0].getImm();
  assert(nzp >= 0b000 && nzp <= 0b111 && "Bad nzp");
  assert(nzp != 0b000 && nzp != 0b111 && "Bad nzp");

  // Branch on equal zero => TBB has zeroed register
  if (nzp == 0b010)
    return &MBB == TBB;
  // Branch on not equal zero => Other block has zeroed register
  if (nzp == 0b101)
    return &MBB != TBB;
  // Any other branch type doesn't guarantee the register will be zero in any
  // block
  return false;
}
