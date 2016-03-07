/**
 * FaultLocalization.h
 *
 * 
 *
 * @author : Si-Mohamed Lamraoui
 * @contact : simo@nii.ac.jp
 * @date : 2012/07/05
 * @copyright : NII 2013
 */

#ifndef _FAULTLOCALIZATION_H
#define _FAULTLOCALIZATION_H

#include <vector>
#include <iostream>
#include <iterator>
#include <algorithm>
//#include <dispatch/dispatch.h>

#include "Options.h"
#include "Utils/Utils.h"
#include "Utils/HittingSet.h"
#include "Profile/ProgramTrace.h"
#include "Profile/ProgramProfile.h"
#include "Logic/Formula.h"
#include "Logic/YicesSolver.h"
#include "Logic/Combine.h"


using namespace llvm;


//============================================================================
class FaultLocalization {
    
private:
    Function *targetFun;
    YicesSolver *solver;
    bool hasArgv;
    Options *options;
    
public:
    FaultLocalization(Function *_targetFun, YicesSolver *_solver,
                       Options *_options) :
                       targetFun(_targetFun), solver(_solver),
                       options(_options) { }
    ~FaultLocalization() { }
    
    void run(Formula *TF, Formula *preCond, Formula *postCond,
             ProgramProfile *prof, Combine::Method combineMethod);
    
    std::vector<SetOfFormulasPtr> allDiagnosis(Formula *TF,
                                               std::vector<ProgramTrace*> traces,
                                               YicesSolver *yices);
    SetOfFormulasPtr allMinMCS(YicesSolver *yices,
                               std::vector<BoolVarExprPtr> &AV,
                               std::map<BoolVarExprPtr, ExprPtr> &AVMap);

private:
    SetOfFormulasPtr avToClauses(SetOfFormulasPtr M,
                                 std::map<BoolVarExprPtr, ExprPtr> AVMap);
    // For debugging
    void checkControlFlow(YicesSolver *solver);
    int  getBlockTransVal(YicesSolver *s, BasicBlock *bb1, BasicBlock *bb2);
    void dumpTransValues(YicesSolver *solver);
    
    
};
//============================================================================

#endif