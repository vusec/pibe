using namespace llvm;
using namespace std;

#include <vector>

vector<string> callbackList = { "vuprof_getTag", "vuprof_setTag"};


namespace llvm {
vector<CallInst*> call_sites;

class CallbackInliner : public ModulePass {

public:
  static char ID;
  CallbackInliner();

  virtual bool runOnModule (Module &M);


};


}
