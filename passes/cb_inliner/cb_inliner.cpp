#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/Casting.h"


#include "cb_inliner.h"

#define PASS_DEBUG_LEVEL 2
#include "llvm_macros.h"




namespace llvm {

/* Constructor */

CallbackInliner::CallbackInliner() : ModulePass(ID) {}

bool CallbackInliner::runOnModule(Module &M){
  int numInlined = 0;

  progress_print("Start inlining callbacks...");

  for (auto curCb = callbackList.begin(),
             endCb = callbackList.end(); 
               curCb != endCb; ++curCb){
       
    Function *F = M.getFunction(*curCb);
    if (F != nullptr){
       for(auto use: F->users()){
         if (isa<CallInst>(use)) {    
            CallInst* instance = cast<CallInst>(use);
            call_sites.push_back(instance);
         }
       }
    }
  }

  for (CallInst *instance: call_sites){   
     InlineFunctionInfo info = InlineFunctionInfo(NULL);
     if (!InlineFunction(CallSite(instance), info)){
                warning_print("Error while inlining call to " << *instance);
     }
     numInlined++;
  }

  progress_print("End inlining callbacks...");
  printf("Inlined %d callbacks\n", numInlined);
  return true;
}



char CallbackInliner::ID = 0;

static RegisterPass<CallbackInliner> X("inline_callbacks", "Inline callbacks deployed by vuprofiler",
                                                    false ,
                                                    true );

}

