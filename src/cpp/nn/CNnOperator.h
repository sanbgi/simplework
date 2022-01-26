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
    //static int createOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnOperator& spOutOp);

public:
    int createOutVar(SNnVariable& spOutVar);

public:
    int initOperator(int nInVars, const SNnVariable pInVars[]);
    int initOutVar(const SDimension& spDimension);

public:
    int solveOneEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int solveTwoEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int createVariable(const SDimension& spDimension, SNnVariable& spOutVar);

public:
    template<typename T> static int createStaticOperator(SNnOperator& spOperator);

protected:
    SDimension m_spDimension;
};

#endif//__SimpleWork_NN_COperator_H__
