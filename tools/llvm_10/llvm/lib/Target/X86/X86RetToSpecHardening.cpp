//===-- X86LoadValueInjectionRetHardening.cpp - LVI RET hardening for x86 --==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// Description: Replaces every `ret` instruction with the sequence:
/// ```
/// pop <scratch-reg>
/// lfence
/// jmp *<scratch-reg>
/// ```
/// where `<scratch-reg>` is some available scratch register, according to the
/// calling convention of the function being mitigated.
///
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/Debug.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include <bitset>
#include <vector>

using namespace llvm;

#define PASS_KEY "x86-spec-ret"
#define DEBUG_TYPE PASS_KEY

STATISTIC(NumRet2SpecFences, "Number of LFENCEs inserted for Ret2Spec mitigation");
STATISTIC(NumRet2SpecFunctionsConsidered, "Number of functions analyzed");
STATISTIC(NumRet2SpecFunctionsMitigated, "Number of functions for which mitigations "
                                 "were deployed");

namespace {

class X86RetToSpecHardeningPass : public MachineFunctionPass {
public:
  X86RetToSpecHardeningPass() : MachineFunctionPass(ID) {}
  StringRef getPassName() const override {
    return "X86 Ret2Spec hardening pass";
  }
  bool runOnMachineFunction(MachineFunction &MF) override;
  MachineBasicBlock *splitBlockBeforeInstr(MachineInstr &MI,
                                           MachineBasicBlock *DestBB);

  static char ID;
};

} // end anonymous namespace

char X86RetToSpecHardeningPass::ID = 0;
static const char ThunkNamePrefix[] = "__llvm_retpoline_";
bool X86RetToSpecHardeningPass::runOnMachineFunction(
    MachineFunction &MF) {
  //LLVM_DEBUG(dbgs() << "***** " << getPassName() << " : " << MF.getName()
  //                  << " *****\n");
  const X86Subtarget *Subtarget = &MF.getSubtarget<X86Subtarget>();
  // TODO add our parameter here
  if (!Subtarget->useReturnToSpec() || !Subtarget->is64Bit())
    return false; // FIXME: support 32-bit
 
  // Would be cool if we could change the retpoline thunk directly in
  // this pass but from my experiments it seems that LLC will not call
  // the pass on retpoline thunks (just modify the thunks directly when
  // we populate them).
  if (MF.getName().startswith(ThunkNamePrefix)){
       return false;
  }

  // Don't skip functions with the "optnone" attr but participate in opt-bisect.
  const Function &F = MF.getFunction();
  if (!F.hasOptNone() && skipFunction(F))
    return false;
  // Use this attribute. It was initially designed for LVI indirect calls
  // but as LVI icalls use Options.td specific CodeGen modifications
  // we cannot use it there. Have it here in case we need to blacklist Ret2Spec functions.
  if (F.hasFnAttribute(Attribute::NoLviIcall)){
    return false;
  }

  ++NumRet2SpecFunctionsConsidered;
  const X86RegisterInfo *TRI = Subtarget->getRegisterInfo();
  const X86InstrInfo *TII = Subtarget->getInstrInfo();
  unsigned ClobberReg = X86::NoRegister;
  std::bitset<X86::NUM_TARGET_REGS> UnclobberableGR64s;
  UnclobberableGR64s.set(X86::RSP); // can't clobber stack pointer
  UnclobberableGR64s.set(X86::RIP); // can't clobber instruction pointer
  UnclobberableGR64s.set(X86::RAX); // used for function return
  UnclobberableGR64s.set(X86::RDX); // used for function return
  SmallVector<MachineInstr*, 8> Rets;
  bool Modified = false;
  for (auto &MBB : MF) {
    if (MBB.empty())
      continue;
    MachineInstr &MI = MBB.back();
    if (MI.getOpcode() == X86::RETQ){
      Rets.push_back(&MI);
    }
  }
  for (MachineInstr *MI : Rets){
    /* Get Return parent*/
    MachineBasicBlock *OrigBB = MI->getParent();
    MachineBasicBlock *NewRetBB =
      MF.CreateMachineBasicBlock(OrigBB->getBasicBlock());
    /* Insert new basic block after the terminator block*/
    MF.insert(++OrigBB->getIterator(), NewRetBB);
    /* Move return to the newBB */
    NewRetBB->splice(NewRetBB->end(), OrigBB, MI->getIterator(), OrigBB->end());
    MachineBasicBlock *BranchCapture = MF.CreateMachineBasicBlock(OrigBB->getBasicBlock());
    /* Add Branch capture between ret basic block and original basic block*/
    MF.insert(++OrigBB->getIterator(), BranchCapture);
    /* Create the return tag */
    MCSymbol *TargetSym = MF.getContext().createTempSymbol();
   
    /* From original basic block call to the return basic block */
    //BuildMI(Entry, DebugLoc(), TII->get(CallOpc)).addSym(TargetSym);
    BuildMI(*OrigBB, OrigBB->end(), DebugLoc(), TII->get(X86::CALL64pcrel32)).addSym(TargetSym);
    /* Trick MIR that we're jumping to the capture */
    OrigBB->addSuccessor(BranchCapture);
    BuildMI(BranchCapture, DebugLoc(), TII->get(X86::PAUSE));
    BuildMI(BranchCapture, DebugLoc(), TII->get(X86::LFENCE));
    BuildMI(BranchCapture, DebugLoc(), TII->get(X86::JMP_1)).addMBB(BranchCapture);
    //BranchCapture->setHasAddressTaken();
    BranchCapture->addSuccessor(BranchCapture);

    //NewRetBB->setHasAddressTaken();
    NewRetBB->setAlignment(Align(16));
    MachineInstrBuilder MIL;
    MIL = addRegOffset(BuildMI(*NewRetBB, *MI, DebugLoc(), TII->get(X86::LEA64r), X86::RSP),
                      X86::RSP, false, 8);

    NewRetBB->front().setPreInstrSymbol(MF, TargetSym);


    ++NumRet2SpecFences;
    Modified = true;
  }

  if (Modified)
    ++NumRet2SpecFunctionsMitigated;
  return Modified;
}

INITIALIZE_PASS(X86RetToSpecHardeningPass, PASS_KEY,
                "X86 Ret2Spec hardener", false, false)

FunctionPass *llvm::createX86RetToSpecHardeningPass() {
  return new X86RetToSpecHardeningPass();
}
