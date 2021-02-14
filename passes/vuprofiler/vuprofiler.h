using namespace llvm;
using namespace std;

#include <vector>

vector<string> whitelistFuncs = { "vuprof_getTag", "vuprof_setTag", "vuprof_epilogue" };

unsigned long long tagGenerator = 0;

namespace llvm {


class VUProfiler : public ModulePass {

public:
  static char ID;
  VUProfiler();

  virtual bool runOnModule (Module &M);


private:
  FunctionCallee setTagFunc, getTagFunc, epilogueFunc;

  int doInit(Module &M);
  void runOnFunction(Function &F);
  void spillTag(Function &F);
  void tagInstruction(Function &F, CallInst *callInst);
  static bool filterCalls(CallInst *);
  
};


}
