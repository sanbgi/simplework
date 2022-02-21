#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"
#include "SNnAtomOperator.h"
#include <map>

using namespace sw;
using namespace std;

typedef int (*FCreateSolver)(SNnSolver& spOutSolver);

class CNnSolver : public CObject, public INnSolver{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    //
    // 创建求解计算器 
    //
    static int solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);

public:
    int createVariable(const SDimension& spDimension, SNnVariable& spOutVar);
    int solveOneEleWise(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int solveTwoEleWise(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int addAtomSolver(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar);

public:
    static map<string, FCreateSolver>& getFactories();
    static int regisetOperator(const char* szOperator, FCreateSolver funCreator);

public:
    template<typename T> static int createStaticSolver(SNnSolver& spOperator) {
        static SNnSolver s_operator = CObject::createObject<T>();
        spOperator = s_operator;
        return 0;
    }
    template<typename T> static int createSolver(SNnSolver& spOperator) {
        CPointer<T> spObj;
        CObject::createObject(spObj);
        spOperator.setPtr(spObj.getPtr());
        return 0;
    }
};

class SNnSolverRegister {
public:
    SNnSolverRegister(const char* szOperator, FCreateSolver funCreator) {
        CNnSolver::regisetOperator(szOperator, funCreator);
    }
};

#endif//__SimpleWork_NN_COperator_H__
