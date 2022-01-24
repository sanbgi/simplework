#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"
#include "CNnVariable.h"
#include "CNnNetwork.h"

using namespace sw;

class CNnOperator;

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnOperator", 211202)

        //
        // 获得计算输出变量
        //
        virtual int getOutVar(SNnVariable& spOutVar) = 0;

        //
        // 获取计算函数
        //
        virtual int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) = 0;

        //
        // 获取对象指针
        //
        virtual CNnOperator* getOpPtr() = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnOperator)


class CNnOperator : public CObject, public INnOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    //
    // 创建求解计算器 
    //
    static int solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp);
    static int solveConv(const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp);
    static int solvePool(const char* szPadding, int nWidth, int nHeight, int nStride, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp);

public:
    virtual int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) = 0;

public:
    int initOperator(int nInVars, const SNnVariable pInVars[]);
    int initOneEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initTwoEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initOutVar(const SDimension& spDimension);

public:
    int getOutVar(SNnVariable& spOutVar);
    CNnOperator* getOpPtr();

protected:
    SDimension m_spDimension;
};

#endif//__SimpleWork_NN_COperator_H__
