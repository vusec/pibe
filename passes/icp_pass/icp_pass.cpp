#include "llvm/Pass.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Casting.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Constants.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Support/Casting.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"
#include "llvm/Analysis/GlobalsModRef.h"
#include "llvm/Analysis/IndirectCallPromotionAnalysis.h"
//#include "llvm/Analysis/IndirectCallSiteVisitor.h"
//#include "llvm/Analysis/OptimizationDiagnosticInfo.h"
//#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CallSite.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/PassSupport.h"
#include "llvm/ProfileData/InstrProf.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Transforms/Instrumentation.h"
//#include "llvm/Transforms/PGOInstrumentation.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Analysis/EHPersonalities.h"

#include "icp_pass.h"

#define PASS_DEBUG_LEVEL 3
#include "llvm_macros.h"

#include <regex>
#include <tuple>

#define EXTENDED_PROFILE_CHECKS


namespace llvm {

ICPPass::ICPPass() : ModulePass(ID) {}

vector<Profile *> ICPPass::doInit(){
  ProfileParser pp;
  return pp.readInProfiles();
}

void ICPPass::dumpBlacklist(vector<int> &vec){
  outs() << "Blacklist:";
  for(vector<int>::const_iterator fi= vec.begin(),
    		      fe = vec.end(); fi != fe; ++fi){
     outs() << *(fi) << ", ";
  }
  outs() << "\n";
}

bool ICPPass::filterCalls(CallInst *callInst)
{
   Function *calee;

   if (callInst->isInlineAsm())
      return false;

   if ((calee = callInst->getCalledFunction()) != nullptr){
       if (calee->isIntrinsic()){
        return false;
       }
   }
   return true;
}

void ICPPass::trackProfiles(Module &M, vector<Profile*> &profiles){
  unsigned int tracked = 0;
  unsigned int index = -1;
#ifdef EXTENDED_PROFILE_CHECKS
  vector<int> blacklist;
#endif

  progress_print("Tracking " << profiles.size() << " profiles");

  for(Profile *profile: profiles){
    Function *child;
    Function *parent;

    totalWeight += profile->weight;
    index++;

    for(string &fAlias : profile->functionAliases){
       child = M.getFunction(fAlias);
       if (child != nullptr && !child->isDeclaration()){
             break;
       }
       child = nullptr;
    }

    if (child == nullptr){
       warning_print("Missing function " << profile->functionAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

    for(string &pAlias : profile->parentAliases){
       parent = M.getFunction(pAlias);
       if (parent != nullptr && !parent->isDeclaration()){
             break;
       }
       parent = nullptr;
    }

    if (parent == nullptr){
       warning_print("Missing parent " << profile->parentAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

    /* Generate parent's coverage list */
    vector<CallInst *> sites = LLVMPassUtils::getInstructionList<CallInst>(*parent, filterCalls);
     
    /* Check if the coverage list has our specified index */
    if ((sites.size() - 1) < profile->coverage){
       warning_print("Coverage of parent does not contain arc:" << profile->parentAliases[0] << "--->" << profile->functionAliases[0]);
       lostWeight += profile->weight;
       continue;
    }

#ifdef EXTENDED_PROFILE_CHECKS
    /* May be slow so only activate this define once then on subsequent runs just filter out the indexes 
       returned in the blacklist */
    if (!LLVMPassUtils::compareLLVMRepresentation(sites[profile->coverage],  profile->llvmRepresentation)){
       warning_print("Current/Past LLVMRepresentation for profile " << index << " do not match");
       blacklist.push_back(index);
       debug_print_element(parent);
       warning_print(profile->llvmRepresentation);
       lostWeight += profile->weight;
       continue;
    }
#endif 
    CallInst *the_call = sites[profile->coverage];
    
    if (profile->type == INDIRECT_CALL){
       if(icpMap.count(the_call) == 0){
            vector<tuple<Function *, Profile *>> Candidates;
            Candidates.push_back(make_tuple(child, profile));
            icpMap[the_call] = Candidates;
       } else {
            icpMap[the_call].push_back(make_tuple(child, profile));
       }
    }
    else if (profile->type == DIRECT_CALL){
       directMap.push_back(make_tuple(the_call, profile));
    }

    tracked++;
  }

  progress_print("Succesfully tracked " << tracked << " out of " << profiles.size() << " profiles");

#ifdef EXTENDED_PROFILE_CHECKS
  dumpBlacklist(blacklist);
#endif

}
static bool isLegalToPromote(Instruction *Inst, Function *F,
                            const char **Reason) {
 // Check the return type.
  Type *CallRetType = Inst->getType();
  if (!CallRetType->isVoidTy()) {
    Type *FuncRetType = F->getReturnType();
    if (FuncRetType != CallRetType &&
        !CastInst::isBitCastable(FuncRetType, CallRetType)) {
      if (Reason)
        *Reason = "Return type mismatch";
      return false;
    }
  }

  // Check if the arguments are compatible with the parameters
  FunctionType *DirectCalleeType = F->getFunctionType();
  unsigned ParamNum = DirectCalleeType->getFunctionNumParams();
  CallSite CS(Inst);
  unsigned ArgNum = CS.arg_size();

  if (ParamNum != ArgNum && !DirectCalleeType->isVarArg()) {
    if (Reason)
      *Reason = "The number of arguments mismatch";
    return false;
  }

  for (unsigned I = 0; I < ParamNum; ++I) {
    Type *PTy = DirectCalleeType->getFunctionParamType(I);
    Type *ATy = CS.getArgument(I)->getType();
    if (PTy == ATy)
      continue;
    if (!CastInst::castIsValid(Instruction::BitCast, CS.getArgument(I), PTy)) {
      if (Reason)
        *Reason = "Argument type mismatch";
      return false;
    }
  }

  return true;
}

static bool getCallRetPHINode(BasicBlock *BB, Instruction *Inst) {
  BasicBlock *From = Inst->getParent();
  for (auto &I : *BB) {
    PHINode *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      continue;
    int IX = PHI->getBasicBlockIndex(From);
    if (IX == -1)
      continue;
    Value *V = PHI->getIncomingValue(IX);
    if (dyn_cast<Instruction>(V) == Inst)
      return true;
  }
  return false;
}

static void insertCallRetPHI(Instruction *Inst, Instruction *CallResult,
                             Function *DirectCallee) {
  if (Inst->getType()->isVoidTy())
    return;

  if (Inst->use_empty())
    return;

  BasicBlock *RetValBB = CallResult->getParent();

  BasicBlock *PHIBB;
  if (InvokeInst *II = dyn_cast<InvokeInst>(CallResult))
    RetValBB = II->getNormalDest();

  PHIBB = RetValBB->getSingleSuccessor();
  if (getCallRetPHINode(PHIBB, Inst))
    return;

  PHINode *CallRetPHI = PHINode::Create(Inst->getType(), 0);
  PHIBB->getInstList().push_front(CallRetPHI);
  Inst->replaceAllUsesWith(CallRetPHI);
  CallRetPHI->addIncoming(Inst, Inst->getParent());
  CallRetPHI->addIncoming(CallResult, RetValBB);
}

static void fixupPHINodeForUnwind(Instruction *Inst, BasicBlock *BB,
                                  BasicBlock *OrigBB,
                                  BasicBlock *IndirectCallBB,
                                  BasicBlock *DirectCallBB) {
  for (auto &I : *BB) {
    PHINode *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      continue;
    int IX = PHI->getBasicBlockIndex(OrigBB);
    if (IX == -1)
      continue;
    Value *V = PHI->getIncomingValue(IX);
    PHI->addIncoming(V, IndirectCallBB);
    PHI->setIncomingBlock(IX, DirectCallBB);
  }
}

static void fixupPHINodeForNormalDest(Instruction *Inst, BasicBlock *BB,
                                      BasicBlock *OrigBB,
                                      BasicBlock *IndirectCallBB,
                                      Instruction *NewInst) {
  for (auto &I : *BB) {
    PHINode *PHI = dyn_cast<PHINode>(&I);
    if (!PHI)
      continue;
    int IX = PHI->getBasicBlockIndex(OrigBB);
    if (IX == -1)
      continue;
    Value *V = PHI->getIncomingValue(IX);
    if (dyn_cast<Instruction>(V) == Inst) {
      PHI->setIncomingBlock(IX, IndirectCallBB);
      PHI->addIncoming(NewInst, OrigBB);
      continue;
    }
    PHI->addIncoming(V, IndirectCallBB);
  }
}

// Add a bitcast instruction to the direct-call return value if needed.
static Instruction *insertCallRetCast(const Instruction *Inst,
                                      Instruction *DirectCallInst,
                                      Function *DirectCallee) {
  if (Inst->getType()->isVoidTy())
    return DirectCallInst;

  Type *CallRetType = Inst->getType();
  Type *FuncRetType = DirectCallee->getReturnType();
  if (FuncRetType == CallRetType)
    return DirectCallInst;

  BasicBlock *InsertionBB;
  if (CallInst *CI = dyn_cast<CallInst>(DirectCallInst))
    InsertionBB = CI->getParent();
  else
    InsertionBB = (dyn_cast<InvokeInst>(DirectCallInst))->getNormalDest();

  return (new BitCastInst(DirectCallInst, CallRetType, "",
                          InsertionBB->getTerminator()));
}

static Instruction *createDirectCallInst(const Instruction *Inst,
                                         Function *DirectCallee,
                                         BasicBlock *DirectCallBB,
                                         BasicBlock *MergeBB) {
  Instruction *NewInst = Inst->clone();
  if (CallInst *CI = dyn_cast<CallInst>(NewInst)) {
    CI->mutateFunctionType(DirectCallee->getFunctionType());
    CI->setCalledFunction(DirectCallee);
  } else {
    // Must be an invoke instruction. Direct invoke's normal destination is
    // fixed up to MergeBB. MergeBB is the place where return cast is inserted.
    // Also since IndirectCallBB does not have an edge to MergeBB, there is no
    // need to insert new PHIs into MergeBB.
    InvokeInst *II = dyn_cast<InvokeInst>(NewInst);
    assert(II);
    II->mutateFunctionType(DirectCallee->getFunctionType());
    II->setCalledFunction(DirectCallee);
    II->setNormalDest(MergeBB);
  }

  DirectCallBB->getInstList().insert(DirectCallBB->getFirstInsertionPt(),
                                     NewInst);

  // Clear the value profile data.
  NewInst->setMetadata(LLVMContext::MD_prof, nullptr);
  CallSite NewCS(NewInst);
  FunctionType *DirectCalleeType = DirectCallee->getFunctionType();
  unsigned ParamNum = DirectCalleeType->getFunctionNumParams();
  for (unsigned I = 0; I < ParamNum; ++I) {
    Type *ATy = NewCS.getArgument(I)->getType();
    Type *PTy = DirectCalleeType->getParamType(I);
    if (ATy != PTy) {
      BitCastInst *BI = new BitCastInst(NewCS.getArgument(I), PTy, "", NewInst);
      NewCS.setArgument(I, BI);
    }
  }

  return insertCallRetCast(Inst, NewInst, DirectCallee);
}

// Create a diamond structure for If_Then_Else. Also update the profile
// count. Do the fix-up for the invoke instruction.
static void createIfThenElse(Instruction *Inst, Function *DirectCallee,
                             BasicBlock **DirectCallBB,
                             BasicBlock **IndirectCallBB,
                             BasicBlock **MergeBB) {
  CallSite CS(Inst);
  Value *OrigCallee = CS.getCalledValue();

  IRBuilder<> BBBuilder(Inst);
  LLVMContext &Ctx = Inst->getContext();
  Value *BCI1 =
      BBBuilder.CreateBitCast(OrigCallee, Type::getInt8PtrTy(Ctx), "");
  Value *BCI2 =
      BBBuilder.CreateBitCast(DirectCallee, Type::getInt8PtrTy(Ctx), "");
  Value *PtrCmp = BBBuilder.CreateICmpEQ(BCI1, BCI2, "");

  Instruction *ThenTerm, *ElseTerm;
  SplitBlockAndInsertIfThenElse(PtrCmp, Inst, &ThenTerm, &ElseTerm,
                                nullptr);
  *DirectCallBB = ThenTerm->getParent();
  (*DirectCallBB)->setName("if.true.direct_targ");
  *IndirectCallBB = ElseTerm->getParent();
  (*IndirectCallBB)->setName("if.false.orig_indirect");
  *MergeBB = Inst->getParent();
  (*MergeBB)->setName("if.end.icp");

  // Special handing of Invoke instructions.
  InvokeInst *II = dyn_cast<InvokeInst>(Inst);
  if (!II)
    return;

  // We don't need branch instructions for invoke.
  ThenTerm->eraseFromParent();
  ElseTerm->eraseFromParent();

  // Add jump from Merge BB to the NormalDest. This is needed for the newly
  // created direct invoke stmt -- as its NormalDst will be fixed up to MergeBB.
  BranchInst::Create(II->getNormalDest(), *MergeBB);
}

static Instruction *promoteIndirectCall(Instruction *Inst,
                                       Function *DirectCallee) {
  assert(DirectCallee != nullptr);
  BasicBlock *BB = Inst->getParent();
  // Just to suppress the non-debug build warning.
  (void)BB;
  //outs() << "\n\n== Basic Block Before ==\n";
  //outs() << *BB << "\n";

  BasicBlock *DirectCallBB, *IndirectCallBB, *MergeBB;
  createIfThenElse(Inst, DirectCallee, &DirectCallBB,
                   &IndirectCallBB, &MergeBB);

  Instruction *NewInst =
      createDirectCallInst(Inst, DirectCallee, DirectCallBB, MergeBB);


  // Move Inst from MergeBB to IndirectCallBB.
  Inst->removeFromParent();
  IndirectCallBB->getInstList().insert(IndirectCallBB->getFirstInsertionPt(),
                                       Inst);

  if (InvokeInst *II = dyn_cast<InvokeInst>(Inst)) {
    // At this point, the original indirect invoke instruction has the original
    // UnwindDest and NormalDest. For the direct invoke instruction, the
    // NormalDest points to MergeBB, and MergeBB jumps to the original
    // NormalDest. MergeBB might have a new bitcast instruction for the return
    // value. The PHIs are with the original NormalDest. Since we now have two
    // incoming edges to NormalDest and UnwindDest, we have to do some fixups.
    //
    // UnwindDest will not use the return value. So pass nullptr here.
    fixupPHINodeForUnwind(Inst, II->getUnwindDest(), MergeBB, IndirectCallBB,
                          DirectCallBB);
    // We don't need to update the operand from NormalDest for DirectCallBB.
    // Pass nullptr here.
    fixupPHINodeForNormalDest(Inst, II->getNormalDest(), MergeBB,
                              IndirectCallBB, NewInst);
  }

  insertCallRetPHI(Inst, NewInst, DirectCallee);

  //outs() << "\n== Basic Blocks After ==\n";
  //outs() << *BB << *DirectCallBB << *IndirectCallBB << *MergeBB << "\n";

  return NewInst;
}

unsigned int ICPPass::tryToPromote(Instruction *Inst, 
                                      const vector<tuple<Function *, Profile *>> &Candidates) 
{
  unsigned int NumPromoted = 0;

  for (auto &C : Candidates) {
     const char *Reason = nullptr;
     if (!isLegalToPromote(Inst, get<0>(C), &Reason)) {
       outs() << Reason << "\n";
       lostWeight += (get<1>(C))->weight;
       //debug_print_element(Inst);
       //debug_print_element(get<0>(C));
       continue;
     }

     Instruction *promotedInst = promoteIndirectCall(Inst, get<0>(C));

     if (isa<CallInst>(promotedInst)){
        NumPromoted++;
        directMap.push_back(make_tuple(cast<CallInst>(promotedInst),  get<1>(C)));
     } 
     else if (isa<BitCastInst>(promotedInst)){
         if (isa<CallInst>(promotedInst->getPrevNode())){
            NumPromoted++;
            directMap.push_back(make_tuple(cast<CallInst>(promotedInst->getPrevNode()),  get<1>(C)));
         } 
         else {
            warning_print("Promotion issue: Instruction immediately after the Bitcast is not a CallInst");
         }
     } 
     else {
         warning_print("Promotion issue: the resulted instruction is neither a CallInst or BitcastInst");
     }
  }

  return NumPromoted;
}

void ICPPass::promoteIndirectProfiles(map<CallInst*, vector<tuple<Function *, Profile *>>> &IcpM){
  unsigned int count = 0;
  progress_print("Promoting calls spanning " << IcpM.size() << " callsites");
  for(auto& icp_pair : IcpM ){
     count += tryToPromote(icp_pair.first, icp_pair.second);
  }
  progress_print("Promoted " << count << " individual call-arc candidates");
}


bool ICPPass::runOnModule(Module &M){

  progress_print("Fetching profiles...");

  vector<Profile*> profiles = doInit();

  trackProfiles(M, profiles);

  promoteIndirectProfiles(icpMap);


  return true;
}


char ICPPass::ID = 0;

static RegisterPass<ICPPass> X("icp_promote", "Indirect call promotion and inlining pass",
                                                    false ,
                                                    true );

}
