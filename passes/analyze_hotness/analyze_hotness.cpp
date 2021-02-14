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
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/BlockFrequencyInfoImpl.h"

#include "analyze_hotness.h"

#define PASS_DEBUG_LEVEL 1
#include "llvm_macros.h"
#include "llvm_utils.h"




namespace llvm {

/* Constructor */

AnalyzeHotness::AnalyzeHotness() : ModulePass(ID) {}
uint64_t minHot = 0;
uint64_t minNormal = 0;
uint64_t maxNormal = 0;
uint64_t maxCold = 0;
uint64_t numTails = 0;
uint64_t maxHot = 0;

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

bool sortFrequencyMap(tuple< uint64_t,uint64_t, float> &i1, tuple< uint64_t,uint64_t, float> &i2) 
{ 

      return ((float)get<2>(i1) < (float)get<2>(i2)); 
} 

bool AnalyzeHotness::runOnModule(Module &M){

  progress_print("Iterating over functions...");

  progress_print("Starting instrumentation...");
  PSI = &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
  if (PSI && PSI->hasProfileSummary()){
      outs() << "Profile summary info available\n";
      //return false;
  } else {
      outs() << "Profile summary no info available\n";
      return false;
  }

  for (auto curFref = M.getFunctionList().begin(),
    	      endFref = M.getFunctionList().end();
    	    	    curFref != endFref; ++curFref){
    Function *F = &(*curFref);
    
    //debug_print_element(F);

    runOnFunction(*F);

    //debug_print_element(F);

  }

  std::sort(FrequencyMap.begin(), FrequencyMap.end(), sortFrequencyMap); 

  for (auto elem: FrequencyMap){

       printf("%.3f  %lu %lu\n", get<2>(elem), get<0>(elem), get<1>(elem));
       //outs() << get<1>(elem) << "|------|" << get<0>(elem) << "\n";
  }

  progress_print("End instrumentation...");
  outs() << "Min hot:" << minHot << "\n";
  outs() << "Max hot:" << maxHot << "\n";
  outs() << "Max normal:" << maxNormal << "\n";
  outs() << "Min normal:" << minNormal << "\n";
  outs() << "Max cold:" << maxCold << "\n";
  outs() << "Num hot:" << numHot << "\n";
  outs() << "Num cold:" << numCold << "\n";
  outs() << "Num normal:" << numNormal << "\n";
  outs() << "Num tails:" << numTails << "\n";
  //outs() << "Num blacklisted:" << numBlacklisted << "\n";

  return true;
}

// This example modifies the program, but does not modify the CFG
void AnalyzeHotness::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.setPreservesCFG();
  AU.addRequired<ProfileSummaryInfoWrapperPass>();
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
}

void AnalyzeHotness::runOnFunction(Function &F){
  // Skip .init.text functions
  if (F.isIntrinsic()){
     return;
  }

  if (F.isDeclaration()){
    return;
  }

  BlockFrequencyInfo *bfi = &getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
  if (!bfi){
      outs() << "No analysis here\n";
      return;
  }
  vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(F, filterCalls);
  uint64_t count = F.getEntryCount().getCount();
 
  for (CallInst *site:sites){
          if (PSI->isHotCallSite(CallSite(site), bfi)){
             numHot++;
             uint64_t TotalWeight = 0;
             if (!site->extractProfTotalWeight(TotalWeight)){
                outs() << "No weight annotated on this profile\n";
             }
             if (minHot == 0){
                minHot = TotalWeight;
             }
             if (minHot > TotalWeight){
                minHot = TotalWeight;
             }
             if (maxHot < TotalWeight){
                maxHot = TotalWeight;
             }
             
             /*
             if (!site->getCalledFunction()->hasFnAttribute(Attribute::NoLviRet)){
                 numBlacklisted++;
                 site->getCalledFunction()->addFnAttr(Attribute::NoLviRet);
             }*/
             if (count != 0){
                 FrequencyMap.push_back(make_tuple(TotalWeight, count, (float)TotalWeight/count));
             }
             Instruction *inst = site->getNextNonDebugInstruction();
	     if (inst != nullptr){
                 if(isa<ReturnInst>(inst)){
                    numTails++;
                 }
             }
             outs() << "HOT_SITE" << F.getName().str()<< "------->" << site->getCalledFunction()->getName().str() << ":" <<  TotalWeight << "\n";
          } else if (PSI->isColdCallSite(CallSite(site), bfi)){
             uint64_t TotalWeight = 0;
             if (!site->extractProfTotalWeight(TotalWeight)){
                 //outs() << "No weight annotated on this profile\n";
             }
             if(maxCold < TotalWeight){
                maxCold = TotalWeight;
             }
 
             numCold++;
          } else {
             uint64_t TotalWeight = 0;
             if (!site->extractProfTotalWeight(TotalWeight)){
                // outs() << "No  weight annotated on this profile\n";
             }
             if (maxNormal < TotalWeight){
                 maxNormal = TotalWeight;
             }

             if (minNormal == 0){
                 minNormal = TotalWeight;
             }
             
             if (minNormal > TotalWeight){
                 minNormal = TotalWeight;
             }
             outs() << "NORMAL_SITE" << F.getName().str()<< "------->" << site->getCalledFunction()->getName().str() << ":" << TotalWeight << "\n";
             numNormal++;
          }
          
          /*
          Function *callee = site->getCalledFunction(); 
          if (!callee->hasFnAttribute(Attribute::NoLviRet)){
                numBlacklisted++;
                callee->addFnAttr(Attribute::NoLviRet);
          }*/
     }
 
}


char AnalyzeHotness::ID = 0;

static RegisterPass<AnalyzeHotness> X("analyze_hotness", "Hotness analyzer pass",
                                                    false ,
                                                    true );

}

