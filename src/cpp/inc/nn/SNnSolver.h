#ifndef __SimpleWork_NnSolver_h__
#define __SimpleWork_NnSolver_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnVariable;

//
// 神经网络解算器，根据参数以及输入变量，返回输出变量，是神经网络推算的基础单元
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnSolver)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnSolver", 211202)

        //
        // 求解
        //
        virtual int solve(const PArg* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnSolver)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnSolver_h__