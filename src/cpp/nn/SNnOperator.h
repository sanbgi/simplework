#ifndef __SimpleWork_NN_SNnOperator_H__
#define __SimpleWork_NN_SNnOperator_H__

#include "nn.h"

using namespace sw;

//
// 计算函数
//
typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

struct PSolveParameter {
    FEval pEvalFun;
    FEval pDeviaFun;
    void* pParameter;
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnOperator", 211202)

        //
        // 解算
        //
        virtual int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) = 0;

        //
        // 获取计算函数
        //
        virtual int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnOperator)


#endif//__SimpleWork_NN_SNnOperator_H__
