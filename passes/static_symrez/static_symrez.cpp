#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
//#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/Casting.h"


#include "llvm/IR/DebugInfoMetadata.h"


#include "static_symrez.h"

#define PASS_DEBUG_LEVEL 3
#include "llvm_macros.h"




namespace llvm {

/* Constructor */

StaticSymbolRezolution::StaticSymbolRezolution() : ModulePass(ID) {}

vector<string> StaticSymbolRezolution::split (string s, string delimiter)
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

void StaticSymbolRezolution::printClashMap(map<string, vector<string>> &cMap){
 std::hash<std::string> str_hash;
 for(auto& clashPair : cMap ){
     outs() << clashPair.first << ":";
     for (string &elem: clashPair.second){
          outs() << elem << "(" << str_hash(elem) << "),";
     }
     outs() << "\n";
 }
}

void StaticSymbolRezolution::remapClashFunctions(Module &M,map<string, vector<string>> &cMap){
 std::hash<std::string> str_hash;
 for(auto& clashPair : cMap ){
     for (string &elem: clashPair.second){
         Function * func = M.getFunction(elem);
         if (llvm::GlobalValue::isLocalLinkage(func->getLinkage())){
             string passphrase = clashPair.first + ";" + func->getSubprogram()->getUnit()->getFilename().str();
             outs() << "PassPhrase:"<< passphrase << "\n";
             string hashV = to_string(str_hash(passphrase));
             outs() << "Hash:" << hashV << "\n";
             string remapFunction = clashPair.first+"."+hashV;
             func->setName(remapFunction);
         } else {
             debug_print(elem << " is not local linkage");
         }
     }
 }
}

bool StaticSymbolRezolution::runOnModule(Module &M){

  progress_print("Start sanitizing asm calls...");

  for (auto curFref = M.getFunctionList().begin(),
    	      endFref = M.getFunctionList().end();
    	    	    curFref != endFref; ++curFref){
    Function *F = &(*curFref);

    if (F->isIntrinsic()){
      continue;
    }    
    
    string funcName = F->getName().str();
    
    if (funcName.find('.') != std::string::npos){

       vector<string> funcParts = split(funcName, ".");

       string baseFunc = funcParts[0];

       if (clashMap.count(baseFunc) == 0){
          vector<string> mappedFunctions;
          Function *Fb = M.getFunction(baseFunc);
          if (Fb != nullptr && !Fb->isDeclaration()){
              mappedFunctions.push_back(baseFunc);
          }
          clashMap[baseFunc] = mappedFunctions;
       } 
       
       clashMap[baseFunc].push_back(funcName);
    }


  }

  //printClashMap(clashMap);
  remapClashFunctions(M, clashMap);
  
  
  progress_print("End sanitizing asm calls...");

  return true;
}



char StaticSymbolRezolution::ID = 0;

static RegisterPass<StaticSymbolRezolution> X("remap_static_clash_symbols", "Sanitize asm .set operations",
                                                    false ,
                                                    true );

}

