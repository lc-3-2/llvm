//==- LC32TestElision.cpp - Test Elision Pass for LC-3.2 --------*- C++ -*--==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "pass/LC32TestElision.h"
#include "LC32CLOpts.h"
#include "LC32InstrInfo.h"
#include "MCTargetDesc/LC32InstPrinter.h"
#include "MCTargetDesc/LC32MCTargetDesc.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
using namespace llvm;
using namespace llvm::lc32::clopts;
#define DEBUG_TYPE "LC32TestElision"

char LC32TestElision::ID = 0;

INITIALIZE_PASS(LC32TestElision, DEBUG_TYPE, "LC-3.2 Test Elision", false,
                false)

STATISTIC(NumTestsElided, "Number of redundant test instruction eliminated");

bool LC32TestElision::runOnMachineFunction(MachineFunction &MF) {

  // Bail if we were told not to execute this pass
  if (this->skipFunction(MF.getFunction()))
    return false;
  if (!EnableTestElision)
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

bool LC32TestElision::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
  LLVM_DEBUG(dbgs() << "Running on MBB " << MBB.getNumber() << '\n');

  // Keep track of the current state of the condition codes, as well as whether
  // we made any changes
  Register CurrentCC = LC32::NoRegister;
  bool MadeChange = false;

  // Iterate over all the instructions in the block
  for (MachineBasicBlock::iterator MBBI = MBB.begin(); MBBI != MBB.end();
       MBBI++) {

    // Compute the condition codes after this instruction
    // Log the output
    Register NextCC = this->transfer(*MBBI, CurrentCC);
    if (NextCC != 0)
      LLVM_DEBUG(dbgs() << "Condition codes hold "
                        << LC32InstPrinter::getRegisterName(NextCC) << " after "
                        << *MBBI);
    else
      LLVM_DEBUG(dbgs() << "Unknown condition codes after " << *MBBI);

    // Update the instruction if needed. Note that we do this after computing
    // the NextCC. This way, we don't have to iterate over all of the produced
    // instructions and risk getting into an infinite loop. Also note that this
    // code modifies MBBI.
    bool UpdateMadeChange = this->update(MBBI, CurrentCC);
    if (UpdateMadeChange) {
      LLVM_DEBUG(dbgs() << "Made change before " << *MBBI);
      NumTestsElided++;
      MadeChange = true;
    }

    // Update CC
    CurrentCC = NextCC;
  }

  // Done
  return MadeChange;
}

Register LC32TestElision::transfer(const MachineInstr &MI, Register CC) {

  // Assert we don't have relaxed instructions at this point
  assert(MI.getOpcode() != LC32::P_FARJSR && "FARJSR generated");
  // Assert that instructions have been eliminated
  assert(MI.getOpcode() != LC32::C_LEA_FRAMEINDEX &&
         "C_LEA_FRAMEINDEX not eliminated");
  assert(MI.getOpcode() != LC32::C_SELECT_CMP_ZERO &&
         "C_SELECT_CMP_ZERO not eliminated");
  assert(MI.getOpcode() != LC32::C_ADJCALLSTACKUP &&
         "C_ADJCALLSTACKUP not eliminated");
  assert(MI.getOpcode() != LC32::C_ADJCALLSTACKDOWN &&
         "C_ADJCALLSTACKDOWN not eliminated");

  // Debug instructions don't do anything
  if (MI.isDebugInstr())
    return CC;

  // Switch on the opcode
  switch (MI.getOpcode()) {
  // Default is to clobber
  default:
    return LC32::NoRegister;

  // The following instructions just set it to the destination register:
  // * ALU
  // * SHF
  // * Move Immediate
  // * Load Constant
  // * Loads
  case LC32::ADDr:
  case LC32::ANDr:
  case LC32::XORr:
  case LC32::LSHFr:
  case LC32::RSHFLr:
  case LC32::RSHFAr:
  case LC32::ANDi:
  case LC32::ADDi:
  case LC32::XORi:
  case LC32::LSHFi:
  case LC32::RSHFLi:
  case LC32::RSHFAi:
  case LC32::C_MOVE_ZERO:
  case LC32::P_LOADCONSTH:
  case LC32::P_LOADCONSTW:
  case LC32::LDB:
  case LC32::LDH:
  case LC32::LDW:
    return MI.getOperand(0).getReg();
  // Special case for `C_LDLR` since its destination is not explicit in the
  // instruction
  case LC32::C_LDLR:
    return LC32::LR;

  // These instructions don't modify the condition codes:
  // * Stores
  // * Indirect Branches
  // * Unconditional Branches
  // * Unannotated Branches
  // * Traps
  case LC32::STB:
  case LC32::STH:
  case LC32::STW:
  case LC32::C_STLR:
  case LC32::JMP:
  case LC32::C_BR_UNCOND:
  case LC32::BR:
  case LC32::TRAP:
    return CC;

  // The `C_BR_CMP_ZERO` instruction does a test and therefore sets the
  // condition codes
  case LC32::C_BR_CMP_ZERO:
    return MI.getOperand(1).getReg();

  // Subroutine calls don't preserve condition codes
  case LC32::JSR:
  case LC32::JSRR:
    return LC32::NoRegister;

  // Return instructions are not modeled as preserving condition codes, even if
  // they technically do. This is because we only consider one machine function
  // at a time.
  case LC32::C_RET:
  case LC32::RTI:
    return LC32::NoRegister;

  // Notionally, LEA doesn't modify the condition codes. However, it may be
  // relaxed into an instruction that does. Therefore, we conservatively assume
  // that it clobbers
  case LC32::LEA:
    return LC32::NoRegister;
  }
}

bool LC32TestElision::update(MachineBasicBlock::iterator &MBBI, Register CC) {
  // Populate variables
  MachineBasicBlock &MBB = *MBBI->getParent();
  MachineFunction &MF = *MBB.getParent();
  DebugLoc DL = MBBI->getDebugLoc();
  const LC32InstrInfo &TII =
      *static_cast<const LC32InstrInfo *>(MF.getSubtarget().getInstrInfo());

  // Only compress `C_BR_CMP_ZERO` instructions
  if (MBBI->getOpcode() != LC32::C_BR_CMP_ZERO)
    return false;

  // Check that the register we're testing is already in the condition codes
  if (MBBI->getOperand(1).getReg() != CC)
    return false;

  // Insert a raw branch
  // For convenience, record the register as an implicit
  MachineInstr *n =
      BuildMI(MBB, MBBI, DL, TII.get(LC32::BR))
          .addImm(MBBI->getOperand(0).getImm())
          .addMBB(MBBI->getOperand(2).getMBB())
          .addReg(MBBI->getOperand(1).getReg(),
                  RegState::Implicit | getRegState(MBBI->getOperand(1)));

  // Erase the old instruction and point to the new one
  MBBI->eraseFromParent();
  MBBI = n->getIterator();
  return true;
}
