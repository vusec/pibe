using namespace llvm;
using namespace std;

#include <vector>
#include "llvm_utils.h"

vector<string> blacklist = { "__cfi_slowpath_diag"};

namespace llvm {


#define INDIRECT_CALL 0
#define DIRECT_CALL 1

class ICPConvert : public ModulePass {

public:
  static char ID;
  ICPConvert();

  virtual bool runOnModule (Module &M);
private:
  vector<Profile *> doInit();
  static bool filterCalls(CallInst *);
  bool compareRepresentation(CallInst *comparator, string llvmRepresentation);
  void dumpBlacklist(vector<int> &);
  void trackProfiles(Module &, vector<Profile*> &);


  void sampleAnalysis(map<Function *, PGOProfileTree *>& pgoMap);


  map<Function *, PGOProfileTree *> tree;
  unsigned long long totalWeight = 0;
  unsigned long long lostWeight = 0;
  
};


}
