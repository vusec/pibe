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
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Debug.h"

#include "bl_lvi.h"

#define PASS_DEBUG_LEVEL 1
#include "llvm_macros.h"
#include "llvm_utils.h"




namespace llvm {

/* Constructor */

BlackListLVI::BlackListLVI() : ModulePass(ID) {}


bool filterCalls(CallInst *callInst)
{
   Function *calee;

   if (callInst->isInlineAsm())
      return false;

   if ((calee = callInst->getCalledFunction()) != nullptr){
       if (calee->isIntrinsic()){
        return false;
       }
       return true;
   }
   return false;
}
int numBlacklisted = 0;

bool BlackListLVI::runOnModule(Module &M){

  progress_print("Iterating over functions...");

  progress_print("Starting instrumentation...");

  for (auto curFref = M.getFunctionList().begin(),
    	      endFref = M.getFunctionList().end();
    	    	    curFref != endFref; ++curFref){
    Function *F = &(*curFref);
    
    //debug_print_element(F);

    runOnFunction(*F);

    //debug_print_element(F);

  }

  progress_print("End instrumentation...");
  outs() << "Num blacklists:" << numBlacklisted << "\n";

  return true;
}

void BlackListLVI::runOnFunction(Function &F){
  // Skip .init.text functions


  bool blacklist = false;
  if (isa<GlobalObject>(&F)){
     GlobalObject *obj = cast<GlobalObject>(&F);
     if (obj->hasSection()){
         if (obj->getSection().str().compare(".init.text") == 0){           
             blacklist = true;
         }
         if (obj->getSection().str().compare(".head.text") == 0){
            blacklist = true;
         }
         /*
         if (obj->getSection().str().compare(".irqentry.text") == 0){
            blacklist = true;
         }
         if (obj->getSection().str().compare(".softirqentry.text") == 0){
             blacklist = true;
         }*/
     }
  }
  
  if (blacklist){
     outs() << F.getName().str() << "\n";
     if (!F.hasFnAttribute(Attribute::NoLviRet)){
            numBlacklisted++;
            F.addFnAttr(Attribute::NoLviRet);
     }
     /*
     vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(F, filterCalls);
     for (CallInst *site:sites){
          Function *callee = site->getCalledFunction(); 
          if (!callee->hasFnAttribute(Attribute::NoLviRet)){
                numBlacklisted++;
                callee->addFnAttr(Attribute::NoLviRet);
          }
     }*/
  }
/*
  if (F.isIntrinsic()){
     return;
  }

  F.addFnAttr(Attribute::NoLviRet);
*/
 
}


char BlackListLVI::ID = 0;

static RegisterPass<BlackListLVI> X("blacklist_lvi", "LVI blacklisting pass",
                                                    false ,
                                                    true );

}

