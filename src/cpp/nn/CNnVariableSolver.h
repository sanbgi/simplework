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
    vector<SNnVariable> arrVars;
    vector<PSolveOperator> arrOperators;
};

class CNnVariableSolver : public CObject, public INnVariableSolver{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariableSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createWeightVariable(const SDimension& spDimension, SNnVariable& spVar);
    int createState(const SDimension& spDimension, SNnState& spVar);
    int solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int loadState(const SNnState spState, SNnVariable& spVar);
    int saveState(const SNnState spState, const SNnVariable& spVar);
    
public:
    static int solveUnit(const SDimension& spInDimension, const SNnUnit& spUnit, PSolveContext* pCtx);
    static int registerSolvedOperator(const SNnOperator& spOp, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar);
};

#endif//__SimpleWork_NN_CNnVariableSolver_H__
