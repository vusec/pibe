#ifndef __LLVM_UTILS_H__
#define __LLVM_UTILS_H__


using namespace llvm;
using namespace std;

#include <vector>
#include <string>
#include <regex>
#include <tuple>

#ifndef DEFAULT_PROFILE_FILE
#define DEFAULT_PROFILE_FILE "/projects/icp_kernel/playground/metadata/5.1.0.workload/Decoded.Profiles.All"
#endif

#define DEFAULT_SAMPLE_FILE "/projects/icp_kernel/playground/metadata/5.1.0.workload/decoded.test"

#define APACHE_PROFILE_FILE "/projects/icp_kernel/playground/metadata/5.1.0.workload/Decoded.Profiles.All.apache"

namespace llvm {

struct Profile {

  vector<string> functionAliases;
  vector<string> parentAliases;
  unsigned int type;
  unsigned long long weight; 
  unsigned int coverage;
  string llvmRepresentation;

  Profile(vector<string> functionAliases, vector<string> parentAliases, unsigned int type,
          unsigned long long weight, unsigned int coverage, string llvmRepresentation){
     this->functionAliases = functionAliases;
     this->parentAliases = parentAliases;
     this->type = type;
     this->weight = weight;
     this->coverage = coverage;
     this->llvmRepresentation = llvmRepresentation;
  };

  void printProfile();
};

struct PGOProfile{
    unsigned long long count;
    Function *F;
    PGOProfile(Function *_F, unsigned long long _count){
       count = _count;
       F = _F;
    };
};

struct PGOProfileTree {
    unsigned long long count;
    unsigned long long headCount;
    map<CallInst*, vector<struct PGOProfile *>> samples;
    PGOProfileTree(){
      count = 0;
      headCount = 0;
   };
};
     

class LLVMPassUtils {

public:
  static bool compareLLVMRepresentation(CallInst *comparator, string llvmRepresentation);
  template<class T>
  static vector<T*> getInstructionList(Function &F,  bool (*)(T*));
};

class ProfileParser {

public: 
  vector<Profile*> readInProfiles(string inputFile = DEFAULT_PROFILE_FILE);

private:
  vector<string> split (string s, string delimiter);
  Profile * parseProfile(string profile);
  unsigned int parseInt(string entry);
  unsigned long long parseLong(string entry);
  vector<string> parseVector(string entry);
};

class LLVMProfileWriter{
public:
  LLVMProfileWriter(){filename = DEFAULT_SAMPLE_FILE;}
  LLVMProfileWriter(string sample_file){filename = sample_file;}
  void writeLLVMSampleProfiles(map<Function *, PGOProfileTree *>& pgoMap);
private:
  void indent(ofstream& OS, int indentation);
  int rebuildInlinedAtChain(ofstream& OS,const DILocation *DIL, unsigned long long totalCount);
  string filename;
};

}


#endif // __LLVM_UTILS_H__
