#include "CNnOperator.h"
#include "CNnOperatorVariable.h"
#include "CUtils.h"
#include "CActivator.h"
#include "CNnVariableSolver.h"
#include <map>

static SCtx sCtx("CNnOperator");
#include "operators/CPlusOperator.h"
#include "operators/CMinusOperator.h"
#include "operators/CMultiplyOperator.h"
#include "operators/CDivideOperator.h"
#include "operators/CJoinOperator.h"
#include "operators/CProductOperator.h"
#include "operators/CSigmodOperator.h"
#include "operators/CTanhOperator.h"
#include "operators/CSoftmaxOperator.h"
#include "operators/CReLUOperator.h"
#include "operators/CConvOperator.h"
#include "operators/CPoolOperator.h"
#include "operators/CStoreStateOperator.h"

typedef int (*FCreateOperator)(SNnOperator& spOutVar);

static map<string, FCreateOperator> s_opFactories = {
    { "storeState", CNnOperator::createStaticOperator<CStoreStateOperator> },
    { "plus", CNnOperator::createStaticOperator<CPlusOperator> },
    { "minus", CNnOperator::createStaticOperator<CMinusOperator> },
    { "multiply", CNnOperator::createStaticOperator<CMultiplyOperator> },
    { "divide", CNnOperator::createStaticOperator<CDivideOperator> },
    { "join", CNnOperator::createStaticOperator<CJoinOperator> },
    { "product", CNnOperator::createStaticOperator<CProductOperator> },
    { "sigmod", CNnOperator::createStaticOperator<CSigmodOperator> },
    { "tanh", CNnOperator::createStaticOperator<CTanhOperator> },
    { "softmax", CNnOperator::createStaticOperator<CSoftmaxOperator> },
    { "relu", CNnOperator::createStaticOperator<CReLUOperator> },
};

int CNnOperator::solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp) {
    map<string, FCreateOperator>::iterator it = s_opFactories.find(szOp);
    if(it != s_opFactories.end()) {
        SNnOperator spOperator;
        if( it->second(spOperator) != sCtx.success() ) {
            return sCtx.error("创建计算器失败");
        }

        SNnVariable spSolvedOut;
        if( spOperator->solve(nInVars, pInVars, spSolvedOut) != sCtx.success() ) {
            return sCtx.error("计算错误");
        }

        return CNnVariableSolver::returnSolvedVar(spOperator, nInVars, pInVars, spSolvedOut, spOutOp);
    }
    return sCtx.error();
}

int CNnOperator::solveConv(const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp) {
    SNnOperator spOperator;
    if( CConvOperator::createOperator(szPadding, nInVars,pInVars, spOperator) != sCtx.success() ) {
        return sCtx.error("计算错误");
    }
    return CNnVariableSolver::returnSolvedVar(spOperator, nInVars, pInVars, spOutOp);
}

int CNnOperator::solvePool(const char* szPadding, int nWidth, int nHeight, int nStride, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutOp) {
    SNnOperator spOperator;
    if( CPoolOperator::createOperator(szPadding, nWidth, nHeight, nStride, nInVars, pInVars, spOperator) != sCtx.success() ) {
        return sCtx.error("计算错误");
    }
    return CNnVariableSolver::returnSolvedVar(spOperator, nInVars, pInVars, spOutOp);
}

int CNnOperator::initOperator(int nInVars, const SNnVariable pInVars[]) {
    if(nInVars > 3) {
        return sCtx.error("目前神经网络的计算输入参数，最大不能超过4个");
    }

    for(int i=0; i<nInVars; i++) {
        SNnInternalVariable spInternalVar = pInVars[i];
        if( !spInternalVar ) {
            return sCtx.error("不认识的参数类型");
        }
    }
    return sCtx.success();
}

int CNnOperator::solveOneEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("参数个数不等于二");
    }

    SDimension spDimension = pInVars[0].dimension();
    return CNnOperatorVariable::createOperatorVariable(spDimension, spOutVar);
}

int CNnOperator::solveTwoEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar){
    if(nInVars != 2) {
        return sCtx.error("参数个数不等于二");
    }

    SDimension spInDimension = pInVars[0].dimension();
    STensor spDimension, spDimension2;
    pInVars[0].dimension()->getVector(spDimension);
    pInVars[1].dimension()->getVector(spDimension2);
    if( !CUtils::isSameDimVector(spDimension, spDimension2) ) {
        return sCtx.error("相加的两个元素维度不一致");
    }

    return CNnOperatorVariable::createOperatorVariable(spInDimension, spOutVar);
}

int CNnOperator::createVariable(const SDimension& spDimension, SNnVariable& spOutVar) {
    return CNnOperatorVariable::createOperatorVariable(spDimension, spOutVar);
}

int CNnOperator::createOutVar(SNnVariable& spOutVar) {
    if(m_spDimension) {
        return CNnOperatorVariable::createOperatorVariable(m_spDimension, spOutVar);
    }
    //如果没有输出维度信息，则表示没有输出信息
    return sCtx.success();
}

int CNnOperator::initOutVar(const SDimension& spDimension) {
    m_spDimension = spDimension;
    return sCtx.success();
}

template<typename Q> int CNnOperator::createStaticOperator(SNnOperator& spOperator) {
    static SNnOperator s_operator = CObject::createObject<Q>();
    spOperator = s_operator;
    return sCtx.success();
}