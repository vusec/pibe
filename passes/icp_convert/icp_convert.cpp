#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/Casting.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IndirectCallPromotionAnalysis.h"
//#include "llvm/Analysis/IndirectCallSiteVisitor.h"
//#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
//#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/PassSupport.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Instrumentation.h"
//#include "llvm/Transforms/PGOInstrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/EHPersonalities.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Transforms/Utils/CallPromotionUtils.h"

#include "icp_convert.h"

#define PASS_DEBUG_LEVEL 3
#include "llvm_macros.h"

#define EXTENDED_PROFILE_CHECKS


namespace llvm {



ICPConvert::ICPConvert() : ModulePass(ID) {}


unsigned int getRecursiveDebugLocation(DebugLoc loc){
   if (DebugLoc InlinedAtDL = loc.getInlinedAt()){
      return getRecursiveDebugLocation(InlinedAtDL);
   }
   else{
      return loc.getLine(); 
   }
}

vector<Profile *> ICPConvert::doInit(){
  ProfileParser pp;
  char *path = getenv("PROFILE_FILE");
  if (path != NULL){
     outs() << "Reader file is " << path << "\n";
     return pp.readInProfiles(path);
  }
  else {
     return pp.readInProfiles();
  }
}

void ICPConvert::dumpBlacklist(vector<int> &vec){
  outs() << "Blacklist:";
  for(vector<int>::const_iterator fi= vec.begin(),
    		      fe = vec.end(); fi != fe; ++fi){
     outs() << *(fi) << ", ";
  }
  outs() << "\n";
}

bool ICPConvert::filterCalls(CallInst *callInst)
{
   Function *calee;

   if (callInst->isInlineAsm())
      return false;

   if ((calee = callInst->getCalledFunction()) != nullptr){
       if (calee->isIntrinsic()){
        return false;
       }
       string caleeName = calee->getName();
       string parentName = callInst->getParent()->getParent()->getName();
       // Eliminate stackprotectors get_random_canary() call  from dup_task_struct effect
       if (parentName.compare("copy_process") == 0){
           if (caleeName.compare("get_random_u64") == 0){
               return false;
           }
       }
       for (auto curCb = blacklist.begin(),
             endCb = blacklist.end(); 
               curCb != endCb; ++curCb){
                    if (caleeName.compare(*curCb) == 0){
                             return false;
                    }           
       }
   }
   return true;
}

    // Add a bitcast instruction to the direct-call return value if needed.
Instruction *mutateReturnValue(Instruction *Inst,
                                          Instruction *DirectCallInst,
                                          Function *DirectCallee) {
  if (Inst->getType()->isVoidTy())
      return DirectCallInst;

  Type *CallRetType = Inst->getType();
  Type *FuncRetType = DirectCallee->getReturnType();
  if (FuncRetType == CallRetType)
        return DirectCallInst;

  BitCastInst *BitCastRet = new BitCastInst(DirectCallInst, CallRetType);
  BitCastRet->insertBefore(Inst);

  return BitCastRet;
}

Instruction *mutateToDirectCall(Instruction *Inst) {
  Instruction *NewInst = Inst->clone();
  CallSite CS(Inst);
  if (!CS){
      outs() << "Error not a call-site\n";
      NewInst->insertBefore(Inst);
      NewInst->eraseFromParent();
      return nullptr;
  }
  Function *DirectCallee = dyn_cast<Function>(CS.getCalledValue()->stripPointerCasts());
  if (!DirectCallee){
      outs() << "Error no direct callee\n";
      Inst->print(outs());
      outs() << "\n";
      NewInst->insertBefore(Inst);
      NewInst->eraseFromParent();
      return nullptr;
  }

  FunctionType *DirectCalleeType = DirectCallee->getFunctionType();
  unsigned ParamNum = DirectCalleeType->getFunctionNumParams();


  /* We first start by checking if parameter types  and return value types
   * of the previous bitcast call instruction are bitcastable to the
   * signature of our mutated function.
   */

  CallSite CheckSite(NewInst);
  for (unsigned I = 0; I < ParamNum; ++I) {
        Type *ATy = CheckSite.getArgument(I)->getType();
        Type *PTy = DirectCalleeType->getParamType(I);
        if (ATy != PTy) {
           if(!ATy->canLosslesslyBitCastTo(PTy))
           {
        	   outs() << "Cast error_type while mutating element:" << *Inst << "\n";
        	   NewInst->insertBefore(Inst);
        	   NewInst->eraseFromParent();
          	   return nullptr;
           }
        }
  }
  Type *CallRetType = NewInst->getType();
  Type *FuncRetType = DirectCallee->getReturnType();
  if (CallRetType != FuncRetType)
  {
     if(!FuncRetType->canLosslesslyBitCastTo(CallRetType))
     {
         outs() << "Cast error_return_type while mutating element:" << *Inst << "\n";
    	 NewInst->insertBefore(Inst);
         NewInst->eraseFromParent();
         return nullptr;
     }
  }

  if (CallInst *CI = dyn_cast<CallInst>(NewInst)) {
      CI->mutateFunctionType(DirectCallee->getFunctionType());
      CI->setCalledFunction(DirectCallee);
  }
  else {
       outs() << "Failed to mutate function signature\n";
       NewInst->insertBefore(Inst);
       NewInst->eraseFromParent();
       return nullptr;
       
  }

  NewInst->insertBefore(Inst);
  CallSite NewCS(NewInst);


  for (unsigned I = 0; I < ParamNum; ++I) {
     Type *ATy = NewCS.getArgument(I)->getType();
     Type *PTy = DirectCalleeType->getParamType(I);
     if (ATy != PTy) {
         BitCastInst *BI = new BitCastInst(NewCS.getArgument(I), PTy, "", NewInst);
         NewCS.setArgument(I, BI);
      }
  }
  Instruction* retBitcast = mutateReturnValue(Inst, NewInst, DirectCallee);
  Inst->replaceAllUsesWith(retBitcast);
  Inst->eraseFromParent();

  return NewInst;
}

unsigned int resolveUnMatchingCall(vector<CallInst *> &sites, unsigned int index, string representation){
     unsigned int range = 4;
     unsigned int min = 0;
     unsigned int max = 0;
     unsigned int i;
     int counter = 0;
     unsigned int poz;
     if (range > index){
         min = 0;
     } else {
        min = index - range;
     }

     if ((sites.size() - 1) < (index+range)){
         max = sites.size() - 1;
     } else {
         max = index+range;
     }
     max = index;
 
     for (i = min; i <= max; i++){
         if (LLVMPassUtils::compareLLVMRepresentation(sites[i],  representation)){
             counter++;
             poz = i;
         }
     }

     if (counter == 1)
          return poz;
     return -1;
 
}

void ICPConvert::trackProfiles(Module &M, vector<Profile*> &profiles){
  unsigned int tracked = 0;
  unsigned int mutated = 0;
  unsigned int totalMutations = 0;
  unsigned int totalBitcasts = 0;
  unsigned int index = -1;
#ifdef EXTENDED_PROFILE_CHECKS
  vector<int> blacklist;
#endif

  progress_print("Tracking " << profiles.size() << " profiles");

  for(Profile *profile: profiles){
    Function *child;
    Function *parent;
    //progress_print("Mutated " << mutated << " profiles");
    totalWeight += profile->weight;
    index++;

    for(string &fAlias : profile->functionAliases){
       child = M.getFunction(fAlias);
       if (child != nullptr && !child->isDeclaration()){
             break;
       }
       child = nullptr;
    }

    if (child == nullptr){
       warning_print("Missing function " << profile->functionAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

    for(string &pAlias : profile->parentAliases){
       parent = M.getFunction(pAlias);
       if (parent != nullptr && !parent->isDeclaration()){
             break;
       }
       parent = nullptr;
    }

    if (parent == nullptr){
       warning_print("Missing parent " << profile->parentAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

    /* Generate parent's coverage list */
    vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(*parent, filterCalls);

    if (sites.size() == 0){
       warning_print("No sites in parent");
       lostWeight += profile->weight;
       continue;
    }
     
    /* Check if the coverage list has our specified index */
    if ((sites.size() - 1) < profile->coverage){
       warning_print("Coverage of parent does not contain arc:" << profile->parentAliases[0] << "--->" << profile->functionAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

    CallInst *the_call = sites[profile->coverage];

#ifdef EXTENDED_PROFILE_CHECKS
    /* May be slow so only activate this define once then on subsequent runs just filter out the indexes 
       returned in the blacklist */
    if (!LLVMPassUtils::compareLLVMRepresentation(sites[profile->coverage],  profile->llvmRepresentation)){
       unsigned int resolution_index = -1;
       resolution_index = resolveUnMatchingCall(sites, profile->coverage, profile->llvmRepresentation);
       if (resolution_index == (unsigned int)-1){
             warning_print("Current/Past LLVMRepresentation for profile " << index << " do not match");
             blacklist.push_back(index);
             //debug_print_element(parent);
             //warning_print(profile->llvmRepresentation);
             lostWeight += profile->weight;
             continue;
       }
       warning_print("We rezolved the call\n");
       the_call = sites[resolution_index];
    }
#endif 
    if (profile->llvmRepresentation.find("bitcast") != std::string::npos){
        totalBitcasts++;
        if (profile->type != DIRECT_CALL){
            outs() << "Some interesting fact" << profile->llvmRepresentation << "\n";
        } if (the_call->getCalledFunction()){
           outs() << "Some interesting fact 2" << profile->llvmRepresentation << "\n";
        }
    }
    if (profile->type == DIRECT_CALL){
        if (!the_call->getCalledFunction()){
           Instruction *aux = nullptr;
           totalMutations++;
           aux = mutateToDirectCall(the_call);
           if (aux != nullptr){
               the_call = cast<CallInst>(aux);
               mutated++;
           }
        }
    }


    // If no profile has been previously created for the parent, then create it
    if (tree.count(parent) == 0){
       PGOProfileTree *profile = new PGOProfileTree;
       tree[parent] = profile;
    }


    if (tree.count(child) == 0){
        PGOProfileTree *profile = new PGOProfileTree;
        tree[child] = profile;
    }
    
    tree[child]->headCount += profile->weight;
    // Create child anyway and mark to mark its entryCount. This might
    // useful for other analyses.
    if (profile->type == INDIRECT_CALL){
       const char *Reason = nullptr;
       if (!isLegalToPromote(CallSite(the_call), child, &Reason)) {
           outs() << "Possible problem with promoting" << child->getName().str() << ":" << profile->llvmRepresentation << "\n";
           continue;
       }
    }
    tree[parent]->count   += profile->weight;

    if (tree[parent]->samples.count(the_call) == 0){
       vector<struct PGOProfile *> sVector;
       tree[parent]->samples[the_call] = sVector;
    }

    tree[parent]->samples[the_call].push_back(new PGOProfile(child, profile->weight));

    tracked++;
  }
   progress_print("Succesfully tracked " << totalBitcasts << " total bitcasts");
  progress_print("Succesfully tracked " << tracked << " out of " << profiles.size() << " profiles");
  progress_print("Succesfully mutated " << mutated << " out of " << totalMutations << " bitcasted calls");

#ifdef EXTENDED_PROFILE_CHECKS
  dumpBlacklist(blacklist);
#endif

}


void ICPConvert::sampleAnalysis(map<Function *, PGOProfileTree *>& pgoMap){
  map<int, int> icpSizes;
  map<Function *, PGOProfileTree *>::iterator it = pgoMap.begin();
  map<int,int>::iterator sizeIt;
  while (it != pgoMap.end()){  
     map<CallInst*, vector<struct PGOProfile *>>::iterator sampleIt = it->second->samples.begin();
     while (sampleIt != it->second->samples.end()){
       int sampleSize = sampleIt->second.size();
       if (icpSizes.count(sampleSize) == 0){
           icpSizes[sampleSize] = 1;
       }else{
           icpSizes[sampleSize]++;
       }
       sampleIt++;
     }
     it++;
  } 
  sizeIt = icpSizes.begin();
  outs() << "\n";
  outs() << "ICP-size(instances):";
  while (sizeIt != icpSizes.end()){
     outs() << sizeIt->first << "(" << sizeIt->second << ")" << ",";
     sizeIt++;
  }
  outs() << "\n";
  
}



bool ICPConvert::runOnModule(Module &M){

  progress_print("Fetching profiles...");

  vector<Profile*> profiles = doInit();

  trackProfiles(M, profiles);

  sampleAnalysis(tree);
 
  LLVMProfileWriter  *pw;
  char *path = getenv("SAMPLE_FILE");
  if (path != NULL){
     outs() << "Writer file is " << path << "\n";
     pw = new LLVMProfileWriter(path);
  } else {
     pw = new LLVMProfileWriter();
  }
  
  pw->writeLLVMSampleProfiles(tree);

  return true;
}


char ICPConvert::ID = 0;

static RegisterPass<ICPConvert> X("icp_convert", "Convert to sampling profile pass",
                                                    false ,
                                                    true );

}
