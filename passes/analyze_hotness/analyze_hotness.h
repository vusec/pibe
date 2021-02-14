using namespace llvm;
using namespace std;

#include <vector>

vector<string> whitelistFuncs = { "vuprof_getTag", "vuprof_setTag", "vuprof_epilogue" };


namespace llvm {


class AnalyzeHotness : public ModulePass {

public:
  static char ID;
  AnalyzeHotness();

  virtual bool runOnModule (Module &M);


private:
  void runOnFunction(Function &F);
  ProfileSummaryInfo *PSI = nullptr;
  vector<tuple< uint64_t, uint64_t, float>> FrequencyMap;

  virtual void getAnalysisUsage(AnalysisUsage &Info) const;
  int numHot = 0;
  int numCold = 0;
  int numNormal = 0;


  
};


}
