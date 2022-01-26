#ifndef __SimpleWork_NN_CNnVariableSolver_H__
#define __SimpleWork_NN_CNnVariableSolver_H__

#include "nn.h"
#include "SNnOperator.h"
#include <vector>

using namespace sw;
using namespace std;

struct PSolveContext {

    struct PSolveOperator {
        //
        // 输入变量数以及对应的下标
        //
        int nInVars;
        int pInVars[4];

        //
        // 输出变量下标
        //
        int iOutVar;

        //
        // 求解函数
        //
        SNnOperator spOperator;
    };
    int iInVar;
    int iOutVar;
    vector<SNnInternalVariable> arrVars;
    vector<PSolveOperator> arrOperators;
};

class CNnVariableSolver : public CObject, public INnVariableSolver{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariableSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createWeightVariable(const SDimension& spDimension, SNnVariable& spVar);
    int createStateVariable(const SDimension& spDimension, SNnVariable& spVar);
    int solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);

public:
    static int solveUnit(const SDimension& spInDimension, const SNnUnit& spUnit, PSolveContext* pCtx);
    static int returnSolvedVar(const SNnOperator& spOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spReturnOut);
};

#endif//__SimpleWork_NN_CNnVariableSolver_H__
