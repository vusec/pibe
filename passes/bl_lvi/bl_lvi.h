using namespace llvm;
using namespace std;

#include <vector>

vector<string> whitelistFuncs = { "vuprof_getTag", "vuprof_setTag", "vuprof_epilogue" };


namespace llvm {


class BlackListLVI : public ModulePass {

public:
  static char ID;
  BlackListLVI();

  virtual bool runOnModule (Module &M);


private:
  void runOnFunction(Function &F);


  
};


}
