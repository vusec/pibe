//=- X86LoadValueInjectionIndirectThunks.cpp - Construct LVI thunks for x86 -=//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
///
/// Description: This pass replaces each indirect call/jump with a direct call
/// to a thunk that looks like:
/// ```
/// lfence
/// jmpq *%r11
/// ```
/// This ensures that if the value in register %r11 was loaded from memory, then
/// the value in %r11 is (architecturally) correct prior to the jump.
///
/// Note: A lot of this code was lifted from X86RetpolineThunks.cpp.
///
//===----------------------------------------------------------------------===//

#include "X86.h"
#include "X86InstrBuilder.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define PASS_KEY "x86-lvi-thunks"
#define DEBUG_TYPE PASS_KEY

static const char R11ThunkName[] = "__x86_indirect_thunk_r11";

namespace {
class X86LoadValueInjectionIndirectThunksPass : public MachineFunctionPass {
public:
  X86LoadValueInjectionIndirectThunksPass() : MachineFunctionPass(ID) {}

  StringRef getPassName() const override {
    return "X86 Load Value Injection (LVI) Indirect Thunks";
  }
  bool doInitialization(Module &M) override;
  bool runOnMachineFunction(MachineFunction &F) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    MachineFunctionPass::getAnalysisUsage(AU);
    AU.addRequired<MachineModuleInfoWrapperPass>();
    AU.addPreserved<MachineModuleInfoWrapperPass>();
  }

  static char ID;

private:
  MachineModuleInfo *MMI;
  const TargetMachine *TM;
  const X86Subtarget *STI;
  const X86InstrInfo *TII;

  bool InsertedThunks;

  void createThunkFunction(Module &M, StringRef Name);
  void populateThunk(MachineFunction &MF, unsigned Reg);
};

} // end anonymous namespace

char X86LoadValueInjectionIndirectThunksPass::ID = 0;

bool X86LoadValueInjectionIndirectThunksPass::doInitialization(Module &M) {
  InsertedThunks = false;
  return false;
}

bool X86LoadValueInjectionIndirectThunksPass::runOnMachineFunction(
    MachineFunction &MF) {
  LLVM_DEBUG(dbgs() << "***** " << getPassName() << " : " << MF.getName()
                    << " *****\n");
  STI = &MF.getSubtarget<X86Subtarget>();
  if (!STI->is64Bit())
    return false; // FIXME: support 32-bit

  // Don't skip functions with the "optnone" attr but participate in opt-bisect.
  const Function &F = MF.getFunction();
  if (!F.hasOptNone() && skipFunction(F))
    return false;

  TM = &MF.getTarget();
  TII = STI->getInstrInfo();
  MMI = &getAnalysis<MachineModuleInfoWrapperPass>().getMMI();
  Module &M = const_cast<Module &>(*MMI->getModule());

  // If this function is not a thunk, check to see if we need to insert
  // a thunk.
  if (MF.getName() != R11ThunkName) {
    // If we've already inserted a thunk, nothing else to do.
    if (InsertedThunks)
      return false;

    // Only add a thunk if one of the functions has the LVI-CFI feature
    // enabled in its subtarget
    if (!STI->useLVIControlFlowIntegrity())
      return false;

    // Otherwise, we need to insert the thunk.
    // WARNING: This is not really a well behaving thing to do in a function
    // pass. We extract the module and insert a new function (and machine
    // function) directly into the module.
    LLVM_DEBUG(dbgs() << "Creating thunk procedure" << '\n');
    createThunkFunction(M, R11ThunkName);
    InsertedThunks = true;
    return true;
  }

  LLVM_DEBUG(dbgs() << "Populating thunk" << '\n');
  populateThunk(MF, X86::R11);
  return true;
}

void X86LoadValueInjectionIndirectThunksPass::createThunkFunction(
    Module &M, StringRef Name) {
  LLVMContext &Ctx = M.getContext();
  auto Type = FunctionType::get(Type::getVoidTy(Ctx), false);
  Function *F =
      Function::Create(Type, GlobalValue::LinkOnceODRLinkage, Name, &M);
  F->setVisibility(GlobalValue::HiddenVisibility);
  F->setComdat(M.getOrInsertComdat(Name));

  // Add Attributes so that we don't create a frame, unwind information, or
  // inline.
  AttrBuilder B;
  B.addAttribute(llvm::Attribute::NoUnwind);
  B.addAttribute(llvm::Attribute::Naked);
  F->addAttributes(llvm::AttributeList::FunctionIndex, B);

  // Populate our function a bit so that we can verify.
  BasicBlock *Entry = BasicBlock::Create(Ctx, "entry", F);
  IRBuilder<> Builder(Entry);

  Builder.CreateRetVoid();

  // MachineFunctions/MachineBasicBlocks aren't created automatically for the
  // IR-level constructs we already made. Create them and insert them into the
  // module.
  MachineFunction &MF = MMI->getOrCreateMachineFunction(*F);
  MachineBasicBlock *EntryMBB = MF.CreateMachineBasicBlock(Entry);

  // Insert EntryMBB into MF. It's not in the module until we do this.
  MF.insert(MF.end(), EntryMBB);
}

void X86LoadValueInjectionIndirectThunksPass::populateThunk(MachineFunction &MF,
                                                            unsigned Reg) {
  // Set MF properties. We never use vregs...
  MF.getProperties().set(MachineFunctionProperties::Property::NoVRegs);

  // Grab the entry MBB and erase any other blocks. O0 codegen appears to
  // generate two bbs for the entry block.
  MachineBasicBlock *Entry = &MF.front();
  Entry->clear();
  while (MF.size() > 1)
    MF.erase(std::next(MF.begin()));

  BuildMI(Entry, DebugLoc(), TII->get(X86::LFENCE));
  BuildMI(Entry, DebugLoc(), TII->get(X86::JMP64r)).addReg(Reg);
  Entry->addLiveIn(Reg);
  return;
}

INITIALIZE_PASS_BEGIN(X86LoadValueInjectionIndirectThunksPass, PASS_KEY,
                      "X86 LVI indirect thunk inserter", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineModuleInfoWrapperPass)
INITIALIZE_PASS_END(X86LoadValueInjectionIndirectThunksPass, PASS_KEY,
                    "X86 LVI indirect thunk inserter", false, false)

FunctionPass *llvm::createX86LoadValueInjectionIndirectThunksPass() {
  return new X86LoadValueInjectionIndirectThunksPass();
}
