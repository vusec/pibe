#include "llvm/Support/Casting.h"
#include "llvm/IR/Function.h"
#include "llvm_utils.h"
#include "llvm/IR/CallSite.h"
#include "llvm/ProfileData/SampleProf.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Support/Debug.h"
#include <iostream>
#include <fstream>

#define PASS_DEBUG_LEVEL 3
#include "llvm_macros.h"

using namespace sampleprof;

namespace llvm {

static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

static string normalizeRepresentation(string reprezentation)
{
    string normalize_vars = regex_replace(reprezentation, regex("%[[:digit:]]+"), string("%0"));
    string normalize_dot =  regex_replace(normalize_vars, regex("\\.[[:digit:]]+"), string(""));
    string result = regex_replace(normalize_dot, regex("#[[:digit:]]+"), string("#0!"));
    trim(result);
    return result.substr(0, result.find("!"));
}

bool LLVMPassUtils::compareLLVMRepresentation(CallInst *comparator, string llvmRepresentation){
  static map<CallInst*, string> decypheredMap;
  string toCompare;
  if (decypheredMap.count(comparator) > 0){
    toCompare = decypheredMap[comparator];
  }
  else{
    string cli;
    raw_string_ostream string_os(cli);
    comparator->print(string_os);
    toCompare = normalizeRepresentation(string_os.str());
    decypheredMap[comparator] = toCompare;
  }
  
  string comparee = normalizeRepresentation(llvmRepresentation);
  if (comparee.compare("") == 0){
       warning_print("Error faulty normalization");
  }


  if (comparee.compare(toCompare) != 0){
      warning_print(comparee);
      warning_print(toCompare);
  }

  return (comparee.compare(toCompare) == 0);
}


template<class T>
vector<T*> LLVMPassUtils::getInstructionList(Function &F, bool (*callback)(T*))
{
  vector<T*> iList;
  for(BasicBlock &BB: F.getBasicBlockList()){
     for(Instruction &II: BB.getInstList()){
        if(isa<T>(II)){
           if (callback != nullptr){
            if(callback(cast<T>(&II)))
             iList.push_back(cast<T>(&II));
           }
           else
             iList.push_back(cast<T>(&II));
        }
     }
  }
  return iList;
}

template vector<CallInst*> LLVMPassUtils::getInstructionList<CallInst>(Function &F, bool (*callback)(CallInst*));

void Profile::printProfile()
{
  debug_print("===Start print profile===");
  string prtr = "";
  for(vector<string>::const_iterator fi= this->functionAliases.begin(),
    		      fe = this->functionAliases.end(); fi != fe; ++fi){

      prtr += "!"+*(fi);
  }
  debug_print("Function aliases:"+prtr);

  prtr = "";
  for(vector<string>::const_iterator fi= this->parentAliases.begin(),
    		      fe = this->parentAliases.end(); fi != fe; ++fi){

      prtr += "!"+*(fi);
  }
  debug_print("Function aliases:"+prtr);

  debug_print("Call type:" + to_string(this->type));
  debug_print("Call weight:" + to_string(this->weight));
  debug_print("Call coverage:" + to_string(this->coverage));
  debug_print("LLVM representation:" + this->llvmRepresentation);

  debug_print("===End print profile===");
  
}

vector<string> ProfileParser::split (string s, string delimiter)
{
  size_t pos_start = 0, pos_end, delim_len = delimiter.length();
  string token;
  vector<string> res;

  while ((pos_end = s.find (delimiter, pos_start)) != string::npos) {
     token = s.substr (pos_start, pos_end - pos_start);
     pos_start = pos_end + delim_len;
     res.push_back (token);
  }

  res.push_back (s.substr (pos_start));
  return res; 
}

vector<string> ProfileParser::parseVector(string entry){
  vector<string> tokens = split(entry, "?");
  return tokens;
}

unsigned int ProfileParser::parseInt(string entry){
  return (unsigned long)(stoul(entry,nullptr,0));
}

unsigned long long ProfileParser::parseLong(string entry){
  return stoull(entry,nullptr,0);
}

Profile *ProfileParser::parseProfile(string profile){
  vector<string> tokens = split(profile, "|");

  vector<string> childAliases = parseVector(tokens[0]);
  vector<string> parentAliases = parseVector(tokens[1]);
  unsigned int type = parseInt(tokens[2]);
  unsigned long long weight = parseLong(tokens[3]);
  unsigned int coverage = parseInt(tokens[4]);
  string llvmRepresentation = tokens[5]; 

  return new Profile(childAliases, parentAliases, type, weight, coverage, llvmRepresentation);
  
}

vector<Profile*> ProfileParser::readInProfiles(string inputFile)
{
  vector<Profile*> profiles;
  fstream in(inputFile, ios::in);
  
  if(!in.is_open()) {
    warning_print("Error while reading profiles - file not found");
    return vector<Profile*>();
  }

  string profileToken;
  while(getline(in, profileToken)){ 
    //debug_print(profileToken);
    Profile *profile = parseProfile(profileToken);
    profiles.push_back(profile);
  }
  in.close();

  return profiles;
}

void LLVMProfileWriter::indent(ofstream& OS, int indentation){
   for (int i = 0; i < indentation ; i++){
       OS << " ";
   }
}

int LLVMProfileWriter::rebuildInlinedAtChain(ofstream& OS,const DILocation *DIL, unsigned long long totalCount){
  int indentation = 1;
  SmallVector<std::pair<LineLocation, StringRef>, 10> S;
  const DILocation *PrevDIL = DIL;

  for (DIL = DIL->getInlinedAt(); DIL; DIL = DIL->getInlinedAt()) {
    S.push_back(std::make_pair(
        LineLocation(sampleprof::FunctionSamples::getOffset(DIL), DIL->getBaseDiscriminator()),
        PrevDIL->getScope()->getSubprogram()->getLinkageName()));

    outs() << "LinkageName" << PrevDIL->getScope()->getSubprogram()->getLinkageName().str() << "-:-" << PrevDIL->getScope()->getSubprogram()->getName().str() << "\n";
    PrevDIL = DIL;
  }

  if (S.size() == 0)
    return 1;

  for (int i = S.size() - 1; i >= 0; i--) {
    indent(OS, indentation);
    LineLocation Loc = S[i].first;
    StringRef callee = S[i].second;

    if (Loc.Discriminator == 0)
      OS << Loc.LineOffset << ": ";
    else
      OS << Loc.LineOffset << "." << Loc.Discriminator << ": ";

    OS << callee.str() << ":" << totalCount << "\n";
    indentation++; 
    
  }
  return indentation;
}

void LLVMProfileWriter::writeLLVMSampleProfiles(map<Function *, PGOProfileTree *>& pgoMap){
   ofstream llvmOutput(filename);
   map<Function *, PGOProfileTree *>::iterator it = pgoMap.begin();

  if (!llvmOutput.is_open()){
    error_print("Could not open sample profile file");
    return;
  }
  
  while (it != pgoMap.end()){
      llvmOutput << it->first->getName().str() << ":" << it->second->count << ":" << it->second->headCount << "\n"; 

      map<CallInst*, vector<struct PGOProfile *>>::iterator sampleIt = it->second->samples.begin();
      while (sampleIt != it->second->samples.end()){
          unsigned long long totalCount = 0;
          for (PGOProfile *elem: sampleIt->second){
              totalCount += elem->count;
          }

          const DILocation *DIL = sampleIt->first->getDebugLoc();

          int indentation = rebuildInlinedAtChain(llvmOutput, DIL, totalCount);

          indent(llvmOutput, indentation);

          uint32_t loc = sampleprof::FunctionSamples::getOffset(DIL);

          if (DIL->getBaseDiscriminator() == 0){
             llvmOutput << loc << ":";

          } else{
             llvmOutput << loc << "." << DIL->getBaseDiscriminator() << ":";
          } 

          llvmOutput << " " << totalCount << " ";
          for (PGOProfile *elem: sampleIt->second){
              Function *sampleCallee = elem->F;
              if (llvm::GlobalValue::isLocalLinkage(sampleCallee->getLinkage())){
                    llvmOutput << " " << sampleCallee->getParent()->getSourceFileName() << ":" << sampleCallee->getName().str();
              }
              else {
                    llvmOutput << " " << sampleCallee->getName().str();
              }
              llvmOutput << ":" << elem->count;
          }    
        
          llvmOutput << "\n";
          sampleIt++;
      }

      it++; 
  }
 
}

}
