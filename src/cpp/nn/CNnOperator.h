#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"

using namespace sw;

//
// 计算函数
//
typedef void (*FEval)(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar);
class CNnOperator;

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
        // 获得计算输出变量
        //
        virtual int createOutVar(SNnVariable& spOutVar) = 0;

        //
        // 获取计算函数
        //
        virtual int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) = 0;

        //
        // 获取对象指针
        //
        //virtual CNnOperator* getOpPtr() = 0;

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
    static int createOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnOperator& spOutOp);

public:
    virtual int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) = 0;
    int createOutVar(SNnVariable& spOutVar);
    CNnOperator* getOpPtr();

public:
    int initOperator(int nInVars, const SNnVariable pInVars[]);
    int initOneEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initTwoEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initOutVar(const SDimension& spDimension);


protected:
    SDimension m_spDimension;
};

#endif//__SimpleWork_NN_COperator_H__
