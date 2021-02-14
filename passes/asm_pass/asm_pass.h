using namespace llvm;
using namespace std;

#include <vector>
#include <string>
#include <iostream>
#include <sstream>

#define WEAK_TOKEN ".weak"
#define SET_TOKEN "\t.set"

#define PARSE_WEAK 0
#define PARSE_SET 1

namespace llvm {


class AsmSanitizer : public ModulePass {

public:
  static char ID;
  AsmSanitizer();

  virtual bool runOnModule (Module &M);
private:
  int stage;
  vector<string> split (string s, string delimiter);


};


}
