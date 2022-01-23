#include "CNnOperator.h"
#include "CUtils.h"
#include "CActivator.h"
#include "CNnWeight.h"
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
#include "operators/CLeRUOperator.h"
#include "operators/CConvOperator.h"
#include "operators/CPoolOperator.h"

typedef int (*FCreateOperator)(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);

static map<string, FCreateOperator> s_opFactories = {
    { "plus", CPlusOperator::createOperator },
    { "minus", CMinusOperator::createOperator },
    { "multiply", CMultiplyOperator::createOperator },
    { "divide", CDivideOperator::createOperator },
    { "join", CJoinOperator::createOperator },
    { "product", CProductOperator::createOperator },
    { "sigmod", CSigmodOperator::createOperator },
    { "tanh", CTanhOperator::createOperator },
    { "softmax", CSoftmaxOperator::createOperator },
    { "leru", CLeRUOperator::createOperator },
};

int CNnOperator::createOperatorVariable(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    map<string, FCreateOperator>::iterator it = s_opFactories.find(szOp);
    if(it != s_opFactories.end()) {
        return it->second(nInVars,pInVars, spOutVar);
    }
    return sCtx.error();
}

int CNnOperator::createConvVariable(const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CConvOperator::createConvVariable(szPadding, nInVars, pInVars, spOutVar);
}

int CNnOperator::createConvVariable(const SNnVariable& spIn, int nWidth, int nHeight, int nConvs, const char* szPadding, SNnVariable& spOutVar) {
    SDimension spDim = spIn.dimension();
    if(spDim.size() < 2) {
        return sCtx.error("卷积操作输入张量至少需要二维，代表高度和宽度");
    }
    int nDim = spDim.size();
    const int* pDimSize = spDim.data();
    int nInputLayers = 1;
    for(int i=2; i<nDim; i++) {
        nInputLayers *= pDimSize[i];
    }

    int pWeightDimSize[4] = { nConvs, nWidth, nHeight, nInputLayers };
    SDimension spWeightDimVector;
    if( SDimension::createDimension(spWeightDimVector, 4, pWeightDimSize) != sCtx.success()) {
        return sCtx.error("创建卷积权重维度张量失败");
    }

    SNnVariable spWeight;
    if( CNnWeight::createWeightVariable(spWeightDimVector, spWeight) != sCtx.success()) {
        return sCtx.error("创建卷积权重变量失败");
    }

    SNnVariable inVars[2] = { spIn, spWeight };
    return CConvOperator::createConvVariable(szPadding, 2, inVars, spOutVar);
}

int CNnOperator::createPoolVariable(const char* szPadding, int nWidth, int nHeight, int nStride, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CPoolOperator::createPoolVariable(szPadding, nWidth, nHeight, nStride, nInVars, pInVars, spOutVar);
}

int CNnOperator::initOperator(int nInVars, const SNnVariable pInVars[]) {
    if(nInVars > 3) {
        return sCtx.error("目前神经网络的计算输入参数，最大不能超过4个");
    }

    m_nInVars = nInVars;
    for(int i=0; i<nInVars; i++) {
        SNnInternalVariable spInternalVar = pInVars[i];
        if( !spInternalVar ) {
            return sCtx.error("不认识的参数类型");
        }
        m_pInVars[i] = spInternalVar; 
    }
    return sCtx.success();
}

int CNnOperator::initOneEleWiseOperator(int nInVars, const SNnVariable pInVars[]) {
    if(nInVars != 1) {
        return sCtx.error("参数个数不等于一");
    }

    m_spDimVector = pInVars[0].dimension();
    return initOperator(nInVars, pInVars);
}

int CNnOperator::initTwoEleWiseOperator(int nInVars, const SNnVariable pInVars[]) {
    if(nInVars != 2) {
        return sCtx.error("参数个数不等于二");
    }

    STensor spDimVector, spDimVector2;
    pInVars[0].dimension()->getVector(spDimVector);
    pInVars[1].dimension()->getVector(spDimVector2);
    if( !CUtils::isSameDimVector(spDimVector, spDimVector2) ) {
        return sCtx.error("相加的两个元素维度不一致");
    }

    m_spDimVector = spDimVector;
    return initOperator(nInVars, pInVars);
}

int CNnOperator::getSubVariables(SNnInternalVariable pSubVariables[4]) {
    for( int i=0; i<m_nInVars; i++ ) {
        pSubVariables[i] = m_pInVars[i];
    }
    return m_nInVars;
}

