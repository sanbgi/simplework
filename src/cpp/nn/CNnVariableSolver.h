#ifndef __SimpleWork_NN_CNnVariableSolver_H__
#define __SimpleWork_NN_CNnVariableSolver_H__

#include "nn.h"
#include "SNnAtomOperator.h"
#include <vector>

using namespace sw;
using namespace std;

//
// 每一步求解参数信息定义
//
struct PNnOperator {
    //
    // 输入变量数以及对应的下标
    //
    int nInVars;
    int pInVars[4];

    //
    // 输出变量下标
    //
    int iOutVar;
};

//
// 求解定义
//
struct PNnSolveGraph {
    int iInVar;
    int iOutVar;
    vector<SNnVariable> arrVars;
    vector<SNnAtomOperator> arrOperators;
    vector<PNnOperator> arrOperatorArgs;

    int toArchive(const SArchive& ar){
        ar.arBlock("iinvar", iInVar);
        ar.arBlock("ioutvar", iOutVar);
        ar.arObjectArray("operators", arrOperators);
        ar.arObjectArray("vars", arrVars);
        ar.arBlockArray<PNnOperator, vector<PNnOperator>>("parameters", arrOperatorArgs);
        return 0;
    }
};

class CNnVariableSolver : public CObject, public INnVariableSolver{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariableSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int loadState(const SNnState spState, SNnVariable& spVar);
    int saveState(const SNnState spState, const SNnVariable& spVar);
    
public:
    static int solveNetwork(const PNnNetwork* pNet, PNnSolveGraph* pCtx);
    static int addAtomOperator(const SNnAtomOperator& spOp, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar);
};

#endif//__SimpleWork_NN_CNnVariableSolver_H__
