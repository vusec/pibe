using namespace llvm;
using namespace std;

#include <vector>
#include "llvm_utils.h"

namespace llvm {

#define INDIRECT_CALL 0
#define DIRECT_CALL 1


class ICPPass : public ModulePass {

public:
  static char ID;
  ICPPass();

  virtual bool runOnModule (Module &M);
private:
  vector<Profile *> doInit();
  static bool filterCalls(CallInst *);
  void dumpBlacklist(vector<int> &);
  void trackProfiles(Module &, vector<Profile*> &);
  void promoteIndirectProfiles(map<CallInst*, vector<tuple<Function *, Profile *>>> &);
  unsigned int tryToPromote(Instruction *Inst, const vector<tuple<Function *, Profile *>> &Candidates); 

  vector<tuple<CallInst *, Profile*>> directMap;
  map<CallInst*, vector<tuple<Function *, Profile *>>> icpMap;
  unsigned long long totalWeight = 0;
  unsigned long long lostWeight = 0;
  
};


}
