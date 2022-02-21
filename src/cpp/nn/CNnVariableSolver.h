#ifndef __SimpleWork_NN_CNnVariableSolver_H__
#define __SimpleWork_NN_CNnVariableSolver_H__

#include "nn.h"
#include "SNnAtomSolver.h"
#include <vector>

using namespace sw;
using namespace std;

//
// 求解定义
//
struct PNnSolver {

    //
    // 每一步求解参数信息定义
    //
    struct PSolveParameter {
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
    int iInVar;
    int iOutVar;
    vector<SNnVariable> arrVars;
    vector<SNnAtomSolver> arrOperators;
    vector<PSolveParameter> arrParameters;
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
    static int solveNetwork(const PNnNetwork* pNet, PNnSolver* pCtx);
    static int addAtomSolver(const SNnAtomSolver& spOp, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar);
};

#endif//__SimpleWork_NN_CNnVariableSolver_H__
