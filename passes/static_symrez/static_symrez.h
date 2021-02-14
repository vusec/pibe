using namespace llvm;
using namespace std;

#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <map>
#include <functional>

namespace llvm {


class StaticSymbolRezolution : public ModulePass {

public:
  static char ID;
  StaticSymbolRezolution();

  virtual bool runOnModule (Module &M);
private:
  int stage;
  vector<string> split (string s, string delimiter);
  void printClashMap(map<string, vector<string>> &);
  map<string, vector<string>> clashMap;
  void remapClashFunctions(Module &M,map<string, vector<string>> &cMap);


};


}
