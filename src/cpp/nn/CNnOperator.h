#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"
#include "CNnVariable.h"
#include "CNnNetwork.h"

using namespace sw;

class CNnOperator : public CNnVariable {
public:
    //
    // 创建求解计算器 
    //
    static int createOperatorVariable(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    static int createConvVariable(const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    static int createPoolVariable(const char* szPadding, int nWidth, int nHeight, int nStride, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    static int createConvVariable(const SNnVariable& spIn, int nWidth, int nHeight, int nConvs, const char* szPadding, SNnVariable& spOutVar);

public:
    virtual int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) = 0;

public:
    int initOperator(int nInVars, const SNnVariable pInVars[]);
    int initOneEleWiseOperator(int nInVars, const SNnVariable pInVars[]);
    int initTwoEleWiseOperator(int nInVars, const SNnVariable pInVars[]);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVOperator; }
    int getSubVariables(SNnInternalVariable pSubVariables[4]);
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType) {
        return nullptr;
    }

private:
    int m_nInVars;
    SNnInternalVariable m_pInVars[4];
};

#endif//__SimpleWork_NN_COperator_H__
