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

#include "llvm/Transforms/IPO/Inliner.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/None.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/AssumptionCache.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/InlineCost.h"
#include "llvm/Analysis/LazyCallGraph.h"
#include "llvm/Analysis/OptimizationRemarkEmitter.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DebugLoc.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Value.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Utils/CallPromotionUtils.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/Transforms/Utils/ImportedFunctionsInliningStatistics.h"
#include "llvm/Transforms/Utils/Local.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"

#include <functional>
#include <sstream>
#include <tuple>
#include <utility>
#include <iostream>
#include <iomanip>

#include "inliner.h"

#define PASS_DEBUG_LEVEL 1
#include "llvm_macros.h"
#include "llvm_utils.h"

#define DEBUG_TYPE "inline"


namespace llvm {

using InlinedArrayAllocasTy = DenseMap<ArrayType *, std::vector<AllocaInst *>>;

/* Constructor */

HotnessInliner::HotnessInliner() : ModulePass(ID) {}
uint64_t minHot = 0;
uint64_t minNormal = 0;
uint64_t maxNormal = 0;
uint64_t maxCold = 0;
uint64_t numTails = 0;

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

bool filterCallsForTotalCost(CallInst *callInst)
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
   return true;
}


int numBlacklisted = 0;

// This example modifies the program, but does not modify the CFG
void HotnessInliner::getAnalysisUsage(AnalysisUsage &AU) const {
  //AU.setPreservesCFG();
  AU.addRequired<ProfileSummaryInfoWrapperPass>();
  AU.addRequired<BlockFrequencyInfoWrapperPass>();
  AU.addRequired<AssumptionCacheTracker>();
  AU.addRequired<TargetLibraryInfoWrapperPass>();
  AU.addRequired<TargetTransformInfoWrapperPass>();
  AU.addRequired<CallGraphWrapperPass>();
  getAAResultsAnalysisUsage(AU);
}

bool InlineRemarkAttribute = true;

/// Return true if the specified inline history ID
/// indicates an inline history that includes the specified function.
static bool InlineHistoryIncludes(
    Function *F, int InlineHistoryID,
    const SmallVectorImpl<std::pair<Function *, int>> &InlineHistory) {
  while (InlineHistoryID != -1) {
    assert(unsigned(InlineHistoryID) < InlineHistory.size() &&
           "Invalid inline history ID");
    if (InlineHistory[InlineHistoryID].first == F)
      return true;
    InlineHistoryID = InlineHistory[InlineHistoryID].second;
  }
  return false;
}

static void setInlineRemark(CallSite &CS, StringRef message) {
  if (!InlineRemarkAttribute)
    return;

  Attribute attr = Attribute::get(CS->getContext(), "inline-remark", message);
  CS.addAttribute(AttributeList::FunctionIndex, attr);
}
/*
static std::basic_ostream<char> &operator<<(std::basic_ostream<char> &R,
                                            const ore::NV &Arg) {
  return R << Arg.Val;
}

static void emit_inlined_into(OptimizationRemarkEmitter &ORE, DebugLoc &DLoc,
                              const BasicBlock *Block, const Function &Callee,
                              const Function &Caller, const InlineCost &IC) {
  ORE.emit([&]() {
    bool AlwaysInline = IC.isAlways();
    StringRef RemarkName = AlwaysInline ? "AlwaysInline" : "Inlined";
    return OptimizationRemark(DEBUG_TYPE, RemarkName, DLoc, Block)
           << ore::NV("Callee", &Callee) << " inlined into "
           << ore::NV("Caller", &Caller) << " with " << IC;
  });
}

template <class RemarkT>
RemarkT &operator<<(RemarkT &&R, const InlineCost &IC) {
  using namespace ore;
  if (IC.isAlways()) {
    R << "(cost=always)";
  } else if (IC.isNever()) {
    R << "(cost=never)";
  } else {
    R << "(cost=" << ore::NV("Cost", IC.getCost())
      << ", threshold=" << ore::NV("Threshold", IC.getThreshold()) << ")";
  }
  if (const char *Reason = IC.getReason())
    R << ": " << ore::NV("Reason", Reason);
  return R;
}

static std::string inlineCostStr(const InlineCost &IC) {
  std::stringstream Remark;
  Remark << IC;
  return Remark.str();
}
*/

/// Return the cost only if the inliner should attempt to inline at the given
/// CallSite. If we return the cost, we will emit an optimisation remark later
/// using that cost, so we won't do so from this function.
static Optional<InlineCost>
shouldInline(CallSite CS, function_ref<InlineCost(CallSite CS)> GetInlineCost) {
  InlineCost IC = GetInlineCost(CS);
  Instruction *Call = CS.getInstruction();
  Function *Callee = CS.getCalledFunction();
  Function *Caller = CS.getCaller();

  if (IC.isAlways()) {
    return IC;
  }

  if (IC.isNever()) {
    return IC;
  }

  if (!IC) {
    return IC;
  }
  /*
  int TotalSecondaryCost = 0;
  if (shouldBeDeferred(Caller, CS, IC, TotalSecondaryCost, GetInlineCost)) {
    return None;
  }*/

  return IC;
}


/* No heuristic implemented yet, just return
   a default treshold for any caller */
int analyzeCallerBudget(Function *F){
   if (F->hasOptNone()){
       return -1;
   }

   return DEFAULT_BUDGET;
}

bool sortWorklist(const CallSite &i1, const CallSite &i2) 
{ 
      uint64_t TotalWeight_1 = 0;
      uint64_t TotalWeight_2 = 0;
      i1.getInstruction()->extractProfTotalWeight(TotalWeight_1);
      i2.getInstruction()->extractProfTotalWeight(TotalWeight_2);
      /*
      if (TotalWeight_1 == TotalWeight_2){
          if (i1.getInstruction()->getParent()->getParent() == i2.getCalledFunction())
              return false;
      }*/
      return (TotalWeight_1 < TotalWeight_2); 
} 

bool HotnessInliner::isElligibleForInlining(CallSite &CS,  BlockFrequencyInfo *BFI){
   if (PSI->isHotCallSite(CS, BFI)){
        return true;
   }
   return false;
}

uint64_t HotnessInliner::analyzeCalleeGain(Function &F){
   uint64_t TotalGain = 0;
   BlockFrequencyInfo *callerBFI = &getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
   vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(F, filterCalls);
   for (auto site : sites){
       CallSite cs(site);
       if (isElligibleForInlining(cs, callerBFI)){
           uint64_t TotalWeight = 0;
           cs.getInstruction()->extractProfTotalWeight(TotalWeight);
           TotalGain += TotalWeight;
       }
   }

   return TotalGain;
}
bool isDefTriviallyDead(Function *F) {
   // Check the linkage
   if (!F->hasLinkOnceLinkage() && !F->hasLocalLinkage() &&
       !F->hasAvailableExternallyLinkage())
     return false;
 
   // Check if the function is used by anything other than a blockaddress.
   for (const User *U : F->users())
     if (!isa<BlockAddress>(U))
       return false;
 
   return true;
 }

bool isDefTriviallyDead2(Function *F)  {
   // Check the linkage
   if (!F->hasLinkOnceLinkage() && !F->hasLocalLinkage())
     return false;
 
   // Check if the function is used by anything other than a blockaddress.
   for (const User *U : F->users())
     if (!isa<BlockAddress>(U))
       return false;
 
   return true;
 }

bool isDefTriviallyDead3(Function *F)  { 
   // Check if the function is used by anything other than a blockaddress.
   for (const User *U : F->users())
       return false;
 
   return true;
 }

void checkForDeadFunctions(vector<Function*> &interval){
    int trivD = 0;
    int trivD2 = 0;
    int trivD3 = 0;
    for(Function *F: interval){
         if (isDefTriviallyDead(F)){
             trivD++;
         }

         if (isDefTriviallyDead2(F)){
             trivD2++;
         }

         if (isDefTriviallyDead3(F)){
             trivD3++;
         }
    }
    outs() << "Dead Analysis:" << trivD << " " << trivD2 << " " << trivD3 << "\n";
}
vector<Function*> deadFuncs;
bool deadContains(Function *F){
    return std::count(deadFuncs.begin(), deadFuncs.end(),F);
}

bool HotnessInliner::inlineCallsImpl(vector<CallSite> &Worklist, CallGraph &CG,
                std::function<AssumptionCache &(Function &)> GetAssumptionCache,
                ProfileSummaryInfo *PSI,
                std::function<TargetLibraryInfo &(Function &)> GetTLI,
                function_ref<InlineCost(CallSite CS)> GetInlineCost,
                function_ref<AAResults &(Function &)> AARGetter,
                ImportedFunctionsInliningStatistics &ImportedFunctionsStats) {


  bool Changed = false;
  SmallVector<std::pair<CallSite, int>, 16> Calls;
  

  if (Worklist.empty()){
    outs() << "Worklist is empty\n";
    return true;
  }

  outs() << "INITIAL_WORKLIST_SIZE:" << Worklist.size() << "\n";

  do {

     // std::sort(Worklist.begin(), Worklist.end(), sortWorklist);
     std::stable_sort(Worklist.begin(), Worklist.end(), sortWorklist); 
      

     CallSite candidate = Worklist.back();
     Worklist.pop_back();

     Function *F = candidate.getCaller();
     Function *Callee = candidate.getCalledFunction();

     if (!Callee || Callee->isDeclaration()){
        outs() << "DECLARATION_ERROR:" << F->getName().str() << " --------> " << Callee->getName().str() << "\n";
        calleeDeclSites.push_back(candidate);
        differentReasons.push_back(candidate);
        continue;
     }

     BlockFrequencyInfo *callerBFI = &getAnalysis<BlockFrequencyInfoWrapperPass>(*F).getBFI();
     /* Check again if the callee is eligible for inlining. Callsites
        that have been previously inlined in a different caller may become
        cold and hence unfit for inlining */
     if (!isElligibleForInlining(candidate, callerBFI)){
        uint64_t TotalWeight = 0;
        candidate.getInstruction()->extractProfTotalWeight(TotalWeight);
        notEligibleSites.push_back(candidate);
        outs() << "UNELIGIBLE_ERROR:" << F->getName().str() << " --------> " << Callee->getName().str() << " LostGain=" << TotalWeight << "\n";
        continue;
     }

     if (callerBudgetMap.count(F) == 0){
        callerBudgetMap[F] = analyzeCallerBudget(F);
        callerWeight[F] = 0;
     }
     /* Caller has noOpt attribute */
     if (callerBudgetMap[F] == -1){
        uint64_t TotalWeight = 0;
        candidate.getInstruction()->extractProfTotalWeight(TotalWeight);
        outs() << "NOPT_ERROR:" << F->getName().str() << " --------> " << Callee->getName().str() << " LostGain=" << TotalWeight << "\n";
        noOptCalls.push_back(candidate);
        differentReasons.push_back(candidate);
        continue;
     }

     InlineCost IC = GetInlineCost(candidate);
     uint64_t toInlineWeight = 0;
     candidate.getInstruction()->extractProfTotalWeight(toInlineWeight);
     if (IC.isNever()) {
       outs() << "NEVER_INLINE:" << candidate.getCaller()->getName().str() << " ------> " << candidate.getCalledFunction()->getName().str() << "\n";
       neverInline.push_back(candidate);
       differentReasons.push_back(candidate);
       continue;
     }

     /* Do not apply the budget euristic for callsites that have negative cost or are always inlinable */
     if (!IC.isAlways() && IC.getCost() > 0) {
       if (callerWeight[F] > callerBudgetMap[F]){
           uint64_t TotalWeight = 0;
           int cost = 0;
           if (!IC.isNever()){
               cost = IC.getCost();
           }
           candidate.getInstruction()->extractProfTotalWeight(TotalWeight);
           outs() << "MAXEDBUDGET_ERROR:" << F->getName().str() << " --------> " << Callee->getName().str() << " LostGain="<< TotalWeight << " Cost=" << cost << "\n";
           maxedBudget.push_back(candidate);
           uint64_t theGain = analyzeCalleeGain(*Callee);
           outs() << "GAIN_INFO:" << theGain << "  " << TotalWeight << "\n";
           continue;
       }
     }


     if (IC.isAlways()) {
        /* If it's always inline don't add any cost penalty to the caller */
        outs() << "ALWAYS_INLINE:" << candidate.getCaller()->getName().str() << " ------> " << candidate.getCalledFunction()->getName().str() << "\n";
     } else {
         
       outs() << "Cost_treshold_balance: Cost=" << IC.getCost() << " Treshold=" <<  IC.getThreshold() << " Callee=" << candidate.getCalledFunction()->getName().str() << "\n";
       if (IC.getCost() > 3000){
        outs() << "TOO Costly:" << candidate.getCaller()->getName().str() << " ------> " << candidate.getCalledFunction()->getName().str() << "\n";
        calleeHeuristic.push_back(candidate);
        continue;
       }
       /*
       uint64_t TotalWeight = 0;
       uint64_t EntryCount = F->getEntryCount().getCount();
       candidate.getInstruction()->extractProfTotalWeight(TotalWeight);
       if (EntryCount != 0 && (TotalWeight <= 20000)){
          if ((float)TotalWeight/EntryCount < 0.9){
               outs() << "No_LOCAL_HOTNESS:" << candidate.getCaller()->getName().str() << " ------> " << candidate.getCalledFunction()->getName().str() << "\n";
               continue;
          }
       }*/
       callerWeight[F] += IC.getCost();
     }
     BlockFrequencyInfo *calleeBFI = &getAnalysis<BlockFrequencyInfoWrapperPass>(*Callee).getBFI();
     InlineFunctionInfo IFI(
         nullptr, &GetAssumptionCache, PSI,
          callerBFI,
          calleeBFI);

     InlineResult IR = InlineFunction(candidate, IFI);
     if (!IR) {
        errorSites.push_back(candidate);
        differentReasons.push_back(candidate);
        outs() << "INLINE_ERROR:" << F->getName().str() << " --------> " << Callee->getName().str() << "\n";
        continue;
      }

     if (!IFI.InlinedCallSites.empty()) {
        for (CallSite &CS : reverse(IFI.InlinedCallSites))
          if (Function *NewCallee = CS.getCalledFunction())
            if (!NewCallee->isDeclaration() && !NewCallee->isIntrinsic()){
              if (isElligibleForInlining(CS, callerBFI)){
                  outs() << "WORKLIST_ADD:" << F->getName().str() << " --------> " << NewCallee->getName().str() << "\n";
                  Worklist.push_back(CS);
              }
            }
     }   
     outs() << "INLINED:" << F->getName().str() << " --------> " << Callee->getName().str() << " Cost=" <<  IC.getCost() << "\n";
     numInlines++;  
     inlinedWeight += toInlineWeight;

     if (!std::count(AnalyzeLiveness.begin(), AnalyzeLiveness.end(),Callee)){
         AnalyzeLiveness.push_back(Callee);
     }

     //uint64_t TotalWeight = 0;
     //candidate.getInstruction()->extractProfTotalWeight(TotalWeight);
     //outs() << "Cost:::::" << TotalWeight << ":::::" << IC.getCost() << "\n";
     //int budget = 

     
     //outs() << elem.getCaller()->getName().str() << ":" << elem.getCalledFunction()->getName().str() << "\n";

  } while(!Worklist.empty());
  checkForDeadFunctions(AnalyzeLiveness);
  int numEliminated = 0;
  /*
  bool change = true;
  while(change){
    change = false;
    for (Function *DeadF: AnalyzeLiveness){
        if (isDefTriviallyDead3(DeadF)){
              DeadF->getBasicBlockList().clear();
              M->getFunctionList().remove(DeadF);
              numEliminated++;
              change = true;
              deadFuncs.push_back(DeadF);
        }
    }
    remove_if(AnalyzeLiveness.begin(), AnalyzeLiveness.end(), deadContains);
    deadFuncs.clear();
  }
  outs() << "NumEliminated:" << numEliminated << "\n";*/
  printInlineStatistics();
  
  return true;
}

bool HotnessInliner::inlineCalls(vector<CallSite> &Worklist) {
   CallGraph &CG = getAnalysis<CallGraphWrapperPass>().getCallGraph();
   ACT = &getAnalysis<AssumptionCacheTracker>();

   Params = getInlineParams();

   auto GetTLI = [&](Function &F) -> TargetLibraryInfo & {
    return getAnalysis<TargetLibraryInfoWrapperPass>().getTLI(F);
   };

   std::function<AssumptionCache &(Function &)> GetAssumptionCache =
        [&](Function &F) -> AssumptionCache & {
      return ACT->getAssumptionCache(F);
    };

   auto GetBFI = [&](Function &F) -> BlockFrequencyInfo & {
      return  *(&getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI());
    };

   auto GetInlineCost = [&](CallSite CS) {
      Function &Callee = *CS.getCalledFunction();
      auto CalleeTTI = &getAnalysis<TargetTransformInfoWrapperPass>().getTTI(Callee);
      return  llvm::getInlineCost(*cast<CallBase>(CS.getInstruction()), Params,
                           *CalleeTTI, GetAssumptionCache, {GetBFI}, PSI,
                          nullptr);
    };
   return inlineCallsImpl(Worklist, CG, GetAssumptionCache, PSI, GetTLI,
       GetInlineCost, LegacyAARGetter(*this),
       ImportedFunctionsStats);
 }

void HotnessInliner::collectSites(Function &F, vector<CallSite> &Worklist){
  // Skip .init.text functions
  if (F.isIntrinsic()){
     return;
  }
  if (F.isDeclaration()){
    return;
  }
  /* Populate Initial Inlining Worklist */
  vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(F, filterCalls);
  BlockFrequencyInfo *BFI = &getAnalysis<BlockFrequencyInfoWrapperPass>(F).getBFI();
  for (CallInst *site:sites){
     CallSite cs(site);
     uint64_t siteWeight = 0;
     cs.getInstruction()->extractProfTotalWeight(siteWeight);
     totalAtBudget++;
     //totalWeightGraph += siteWeight;
     if (isElligibleForInlining(cs, BFI)){
         totalWeightAtBudget += siteWeight;
         hotAtBudget++;
         Worklist.push_back(cs);
     } else {
         if (siteWeight >= 1){
            coldAtBudget++;
         }
     }
  }
  vector<CallInst *> sitesT = LLVMPassUtils::getInstructionList<CallInst>(F, filterCallsForTotalCost);

  for (CallInst *site:sitesT){
     CallSite cs(site);
     uint64_t siteWeight = 0;
     cs.getInstruction()->extractProfTotalWeight(siteWeight);
     totalWeightGraph += siteWeight;
  }
  
 
}
uint64_t computeLoss(vector<CallSite>&  sites){
    uint64_t lostWeight = 0;
    for (CallSite cs : sites){
          uint64_t toInlineWeight = 0;
          cs.getInstruction()->extractProfTotalWeight(toInlineWeight);
          lostWeight += toInlineWeight;
    }
    return lostWeight;
}

string prround(float var) 
{ 
    // we use array of chars to store number 
    // as a string. 
    char str[40];  
    memset(str, 0, 40);
    // Print in string the value of var  
    // with two decimal point 
    sprintf(str, "%.6f", var); 
  
    // scan string value in var  
    //#sscanf(str, "%f", &var);  
  
    return str;  
} 

void HotnessInliner::computeFractions(uint64_t loss){
   outs() << "Overall:" << prround(((float)loss/totalWeightGraph)*100) << " Budget:" <<  prround(((float)loss/totalWeightAtBudget)*100) << "\n";
}
void HotnessInliner::printInlineStatistics(){
    /* Some issue fix */
    if (13889255751 != totalWeightGraph)
           totalWeightGraph = 13889255751;
 
    outs() << "TotalWeight:" << totalWeightGraph << "\n";
    outs() << "TotalBudgetWeight:" << totalWeightAtBudget << "\n";
    outs() << "TotalAtBudget:" << totalAtBudget << "\n";
    outs() << "HotAtBudget:" << hotAtBudget << "\n";
    outs() << "ColdAtBudget:" << coldAtBudget << "\n";
    outs() << "InlinedNum:" << numInlines << "\n";
    outs() << "InlinedWeight:" << inlinedWeight << "\n";
    outs() << "InlinedWeightPercent:" <<  prround(((float)inlinedWeight/totalWeightGraph)*100) << "\n";

    outs() << "\n" << "\n";
    outs() << "SitesMissed(H1):" << notEligibleSites.size() << "\n";
    outs() << "SitesMissedPercent(H1):" << prround(((float)notEligibleSites.size()/hotAtBudget)*100) << "\n";
    outs() << "WeightLost(H1):" << computeLoss(notEligibleSites) << "\n";  
    outs() << "WeightLostPercent(H1):" << prround(((float)computeLoss(notEligibleSites)/totalWeightGraph)*100) << "\n";

    outs() << "\n" << "\n";
    outs() << "SitesMissed(H2):" << maxedBudget.size() << "\n";
    outs() << "SitesMissedPercent(H2):" << prround(((float)maxedBudget.size()/hotAtBudget)*100) << "\n";
    outs() << "WeightLost(H2):" << computeLoss(maxedBudget) << "\n";  
    outs() << "WeightLostPercent(H2):" << prround(((float)computeLoss(maxedBudget)/totalWeightGraph)*100) << "\n";

    outs() << "\n" << "\n";
    outs() << "SitesMissed(H3):" << calleeHeuristic.size() << "\n";
    outs() << "SitesMissedPercent(H3):" << prround(((float)calleeHeuristic.size()/hotAtBudget)*100) << "\n";
    outs() << "WeightLost(H3):" << computeLoss(calleeHeuristic) << "\n";  
    outs() << "WeightLostPercent(H3):" << prround(((float)computeLoss(calleeHeuristic)/totalWeightGraph)*100) << "\n";

    outs() << "\n" << "\n";
    outs() << "WeightLost(H1+H2+H3):" << (computeLoss(notEligibleSites)+computeLoss(maxedBudget)+computeLoss(calleeHeuristic)) << "\n";  
    outs() << "WeightLostPercent(H1+H2+H3):" << prround(((float)(computeLoss(notEligibleSites)+computeLoss(maxedBudget)+computeLoss(calleeHeuristic))/totalWeightGraph)*100) << "\n";
    outs() << "\n" << "\n";
    outs() << "SitesMissed(other):" << differentReasons.size() << "\n";
    outs() << "SitesMissedPercent(other):" << prround(((float)differentReasons.size()/hotAtBudget)*100) << "\n";
    outs() << "WeightLost(other):" << computeLoss(differentReasons) << "\n";  
    outs() << "WeightLostPercent(other):" << prround(((float)computeLoss(differentReasons)/totalWeightGraph)*100) << "\n";

    outs() << "\n" << "\n" << "\n";
    outs() << "Sites lost(errors):" << errorSites.size() << "\n";
    outs() << "Weight lost(errors):" << computeLoss(errorSites) << "\n";
    computeFractions(computeLoss(errorSites));
    outs() << "Sites lost(decls):" << calleeDeclSites.size() << "\n";
    outs() << "Weight lost(decls):" << computeLoss(calleeDeclSites) << "\n";
    computeFractions(computeLoss(calleeDeclSites));
    outs() << "Sites lost(noopt):" << noOptCalls.size() << "\n";
    outs() << "Weight lost(noopt):" << computeLoss(noOptCalls) << "\n";
    computeFractions(computeLoss(noOptCalls));
    outs() << "Sites lost(neverinline):" << neverInline.size() << "\n";
    outs() << "Weight lost(neverinline):" << computeLoss(neverInline) << "\n";
    computeFractions(computeLoss(neverInline));
    
}

bool HotnessInliner::runOnModule(Module &Mo){

  progress_print("Starting instrumentation...");
  PSI = &getAnalysis<ProfileSummaryInfoWrapperPass>().getPSI();
  if (PSI && PSI->hasProfileSummary()){
      outs() << "Profile summary info available\n";
  } else {
      outs() << "Profile summary no info available\n";
      return false;
  }
  M = &Mo;
  vector<CallSite> CallSites;

  for (auto curFref = M->getFunctionList().begin(),
    	      endFref = M->getFunctionList().end();
    	    	    curFref != endFref; ++curFref){
    Function *F = &(*curFref);

    collectSites(*F, CallSites);

  }
  inlineCalls(CallSites);

  progress_print("End instrumentation...");

  return true;
}


char HotnessInliner::ID = 0;

static RegisterPass<HotnessInliner> X("hotness_inliner", "Hotness inliner pass",
                                                    false ,
                                                    true );

}

