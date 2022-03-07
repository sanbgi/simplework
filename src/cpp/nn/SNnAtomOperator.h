#ifndef __SimpleWork_NN_SNnAtomOperator_H__
#define __SimpleWork_NN_SNnAtomOperator_H__

#include "nn.h"

using namespace sw;

//
// 计算函数
//
typedef void (*FBatchEval)(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar);
typedef void (*FBatchDevia)(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);

//
// 求解上下文
//
struct PSolveCtx {
    unsigned int idType;
    enum Sdk{
        CPU,
        OpenCL,
        CUDA
    }eSdk;
};

// 
// 求解函数
//
struct PSolveFunc {
    FBatchEval pEvalFun;
    FBatchDevia pDeviaFun;
    int nCustomerRange;
    int nParamterSize;
    void* pParameterData;
    enum {
        PBatchAndOut,
        PBatch,
        POut,
        PCustomer,
        PNo
    }eClRange;
};

//
// 神经网络原子计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnAtomOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnAtomOperator", 211202)

        //
        // 名字
        //
        virtual const char* getName() = 0;

        //
        // 准备计算参数
        //
        virtual int prepareSolver(const PSolveCtx solveCtx, PSolveFunc& solveParameter) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnAtomOperator)


#endif//__SimpleWork_NN_SNnAtomOperator_H__
