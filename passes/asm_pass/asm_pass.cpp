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


#include "asm_pass.h"

#define PASS_DEBUG_LEVEL 2
#include "llvm_macros.h"




namespace llvm {

/* Constructor */

AsmSanitizer::AsmSanitizer() : ModulePass(ID) {}

vector<string> AsmSanitizer::split (string s, string delimiter)
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

bool AsmSanitizer::runOnModule(Module &M){

  progress_print("Start sanitizing asm calls...");

  string s = M.getModuleInlineAsm();
  istringstream input(s);
  ostringstream output;

  stage = PARSE_WEAK;
  string previous_token;
  for (std::string line; std::getline(input, line); ) {
     if (stage == PARSE_WEAK){
       if (line.compare(0, strlen(WEAK_TOKEN), WEAK_TOKEN) == 0){
         stage = PARSE_SET;
         previous_token = line; 
       } else {
         output << line << "\n"; // replace this with a stringstream
       }
     }
     else if (stage == PARSE_SET) {  
       if (line.compare(0, strlen(SET_TOKEN), SET_TOKEN) == 0){
          vector<string> tokens = split(line,",");
          if (tokens.size() >= 2 && (tokens[1].compare("sys_ni_syscall") == 0)){
            vector<string> nameTokens = split(previous_token, " ");
            string sysName = nameTokens[1];
            Function *sysFunc = M.getFunction(sysName);

            // Remove inline asm which initializes system calls to
            // sys_ni_syscall. Workaround for syscall promotions being 
            // lowered to calls to sys_ni_syscall instead of their
            // definition within the blob. 
            if (sysFunc != nullptr && !sysFunc->isDeclaration()){
               outs() << previous_token << "\n";
               outs() << line << "\n";
            } else {
               output << previous_token << "\n";
               output << line << "\n";  
            }
          } else {
             // output to the string stream
             output << previous_token << "\n";
             output << line << "\n";
          }
          
       } else {
          output << previous_token << "\n";
          output << line << "\n";
       }
       stage = PARSE_WEAK;
     }
  }

  string out_string = output.str();
  //outs() << out_string;
  M.setModuleInlineAsm(out_string);
  progress_print("End sanitizing asm calls...");

  return true;
}



char AsmSanitizer::ID = 0;

static RegisterPass<AsmSanitizer> X("sanitize_asm", "Sanitize asm .set operations",
                                                    false ,
                                                    true );

}

