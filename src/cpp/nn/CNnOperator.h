#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"
#include "SNnOperator.h"

using namespace sw;

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

public:
    int initOperator(int nInVars, const SNnVariable pInVars[]);
    int initOneEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initTwoEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initOutVar(const SDimension& spDimension);


protected:
    SDimension m_spDimension;
};

#endif//__SimpleWork_NN_COperator_H__
