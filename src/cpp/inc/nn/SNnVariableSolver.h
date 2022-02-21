#ifndef __SimpleWork_NnVariableSolver_h__
#define __SimpleWork_NnVariableSolver_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnState;

//
// 解算器
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnVariableSolver, "sw.nn.NnVariableSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnVariableSolver", 211202)

        //
        // 加载状态值
        //
        virtual int loadState(const SNnState spState, SNnVariable& spVar) = 0;

        //
        // 保存状态值
        //
        virtual int saveState(const SNnState spState, const SNnVariable& spVar) = 0;

        //
        // 运算求解
        //
        virtual int solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SNnVariableSolver& getSolver() {
        static SNnVariableSolver g_solver = SObject::createObject<SNnVariableSolver>();
        return g_solver;
    } 

SIMPLEWORK_INTERFACECLASS_LEAVE(NnVariableSolver)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnVariableSolver_h__