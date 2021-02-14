using namespace llvm;
using namespace std;

#include "llvm/Analysis/InlineCost.h"
#include <vector>

vector<string> whitelistFuncs = { "vuprof_getTag", "vuprof_setTag", "vuprof_epilogue" };


namespace llvm {

#define DEFAULT_BUDGET 12000

class HotnessInliner : public ModulePass {

public:
  static char ID;
  HotnessInliner();

  virtual bool runOnModule (Module &M);


private:
  void collectSites(Function &F, vector<CallSite> &Worklist);
  bool inlineCallsImpl(vector<CallSite> &Worklist, CallGraph &CG,
                std::function<AssumptionCache &(Function &)> GetAssumptionCache,
                ProfileSummaryInfo *PSI,
                std::function<TargetLibraryInfo &(Function &)> GetTLI,
                function_ref<InlineCost(CallSite CS)> GetInlineCost,
                function_ref<AAResults &(Function &)> AARGetter,
                ImportedFunctionsInliningStatistics &ImportedFunctionsStats);
  bool inlineCalls(vector<CallSite> &Worklist);
  bool isElligibleForInlining(CallSite &CS,  BlockFrequencyInfo *BFI);
  void printInlineStatistics();
  uint64_t analyzeCalleeGain(Function &F);
  void computeFractions(uint64_t loss);
  Module *M;
 
protected:
  AssumptionCacheTracker *ACT;
  ProfileSummaryInfo *PSI = nullptr;
  std::function<const TargetLibraryInfo &(Function &)> GetTLI;
  ImportedFunctionsInliningStatistics ImportedFunctionsStats;
  InlineParams Params;
 
  vector<CallSite>  noOptCalls;
  vector<CallSite>  maxedBudget;
  vector<CallSite>  calleeHeuristic;
  vector<CallSite>  errorSites;
  vector<CallSite>  calleeDeclSites;
  vector<CallSite>  differentReasons;
  vector<CallSite>  notEligibleSites;
  vector<CallSite>  neverInline;
  vector<Function*> AnalyzeLiveness;
  map<Function *, int> callerWeight;
  uint64_t totalWeightGraph = 0;
  uint64_t totalWeightAtBudget = 0;
  uint64_t inlinedWeight = 0;
  int totalAtBudget = 0;
  int hotAtBudget = 0;
  int coldAtBudget = 0;
  int numInlines = 0;
  /* Perhaps it would be good in some cases if we give higher budget to some specific callers, keep occurence of this callers here */
  map<Function *, int> callerBudgetMap;

  virtual void getAnalysisUsage(AnalysisUsage &Info) const;
  int numHot = 0;
  int numCold = 0;
  int numNormal = 0;


  
};


}
