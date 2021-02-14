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

#include "vuprofiler.h"

#define PASS_DEBUG_LEVEL 1
#include "llvm_macros.h"
#include "llvm_utils.h"




namespace llvm {

/* Constructor */

VUProfiler::VUProfiler() : ModulePass(ID) {}

/* Initialize callbacks used by the profiler */

int VUProfiler::doInit(Module &M){
  Type *ret_setTag, *ret_getTag, *ret_epilogue;
  vector<Type *> args_setTag;
  vector<Type *> args_epilogue;

  // Create signature void vuprof_setTag(u64)

  ret_setTag = Type::getVoidTy(M.getContext());
  args_setTag.push_back(Type::getInt64Ty(M.getContext()));

  setTagFunc = M.getOrInsertFunction("vuprof_setTag", FunctionType::get(ret_setTag, args_setTag, false));

  if (isa<BitCastInst>(setTagFunc.getCallee())){
     return -1;
  }

  // Create signature u64 vuprof_getTag(void)
  ret_getTag = Type::getInt64Ty(M.getContext());
  getTagFunc = M.getOrInsertFunction("vuprof_getTag", FunctionType::get(ret_getTag, false));

  if (isa<BitCastInst>(getTagFunc.getCallee())){
     return -1;
  }


  // Create signature vuprof_epilogue(unsigned long long *)

  ret_epilogue = Type::getVoidTy(M.getContext());
  args_epilogue.push_back(Type::getInt64Ty(M.getContext()));

  epilogueFunc = M.getOrInsertFunction("vuprof_epilogue", FunctionType::get(ret_epilogue, args_epilogue, false));

  if (isa<BitCastInst>(epilogueFunc.getCallee())){
     return -1;
  }

  return 0;
}

bool VUProfiler::runOnModule(Module &M){

  progress_print("Fetching callbacks...");

  if (doInit(M)){
     error_print("Callback get error");
     return false;
  }

  progress_print("Starting instrumentation...");

  for (auto curFref = M.getFunctionList().begin(),
    	      endFref = M.getFunctionList().end();
    	    	    curFref != endFref; ++curFref){
    Function *F = &(*curFref);
    
    debug_print_element(F);

    runOnFunction(*F);

    debug_print_element(F);

  }

  progress_print("End instrumentation...");
  progress_print("Last generated tag is " << tagGenerator);

  return true;
}

void VUProfiler::runOnFunction(Function &F){
  vector<CallInst*> sites;

  // Skip .init.text functions
  if (isa<GlobalObject>(&F)){
     GlobalObject *obj = cast<GlobalObject>(&F);
     if (obj->hasSection()){
         if (obj->getSection().str().compare(".init.text") == 0){
             return;
         }
     }
  }
  // Skip declarations
  if (F.isDeclaration()){
     return;
  }

  // Skip llvm intrinsic functions from instrumentation
  if (F.isIntrinsic()){
      return;
  }
 
  // Skip whitelist functions from instrumentation
  for(vector<string>::const_iterator fi= whitelistFuncs.begin(),
    		      fe = whitelistFuncs.end(); fi != fe; ++fi){

      if (F.getName().compare(*fi) == 0){
         return;
      }
  }


  sites = LLVMPassUtils::getInstructionList<CallInst>(F, filterCalls);
  if(!sites.empty()){
     for(vector<CallInst*>::const_iterator fi= sites.begin(),
    		      fe = sites.end(); fi != fe; ++fi){

         //debug_print_element((*fi));
         // Add a call to setTag before each nested call instruction and increment the tag generator
         tagInstruction(F, (*fi));
     }
  }

  // Add a call to the profiling callback
  spillTag(F);

}

// Passes the tag as a parameter to the vuprof_epilogue function
void VUProfiler::spillTag(Function &F){
   Instruction *ii;
   vector<Value *> epilogueArgs;
   Function::iterator b_iterator = F.begin();
   BasicBlock *bb = &(*b_iterator);
   
   /* Skip all AllocaInstructions */
   for(BasicBlock::iterator it = bb->begin(), ie = bb->end(); 
                                      it != ie; ++it){
        ii = &(*it);
    	if(!isa<AllocaInst>(ii)){
            break;
    	}
   }
   /* Add a call to the vuprof_getTag after the last AllocaInst */
   CallInst *tagCall = CallInst::Create(getTagFunc.getCallee(), "", ii);

   /* Create and Add a call to vuprof_epilogue after the call to vuprof_getTag */
   epilogueArgs.push_back(tagCall);

   CallInst::Create(epilogueFunc.getCallee(), epilogueArgs, "", ii);
}

void VUProfiler::tagInstruction(Function &F, CallInst *callInst){
   vector<Value *> setTag_params;
   LLVMContext &ctx = F.getParent()->getContext();
   setTag_params.push_back(ConstantInt::get(ctx, APInt(64, ++tagGenerator)));

   CallInst::Create(setTagFunc, setTag_params, "", callInst);
}

bool VUProfiler::filterCalls(CallInst *callInst)
{
   Function *calee;

   if (callInst->isInlineAsm())
      return false;

   if ((calee = callInst->getCalledFunction()) != nullptr){
       if (calee->isIntrinsic()){
        return false;
       }
   }
   return true;
}
    	

char VUProfiler::ID = 0;

static RegisterPass<VUProfiler> X("vuprofiler_instrument", "VUProfiler tagging/profiling pass",
                                                    false ,
                                                    true );

}

