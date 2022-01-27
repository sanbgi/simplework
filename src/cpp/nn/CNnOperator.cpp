#include "CNnOperator.h"
#include "CNnOperatorVariable.h"
#include "CUtils.h"
#include "CActivator.h"
#include "CNnVariableSolver.h"
#include <map>

static SCtx sCtx("CNnOperator");
map<string, FCreateOperator>& CNnOperator::getFactories() {
    static map<string, FCreateOperator> s_opFactories = {
    };
    return s_opFactories;
}

int CNnOperator::regisetOperator(const char* szOperator, FCreateOperator funCreator) {
    getFactories()[szOperator] = funCreator;
    return sCtx.success();
}

int CNnOperator::solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    map<string, FCreateOperator>::iterator it = getFactories().find(szOp);
    if(it != getFactories().end()) {
        SNnOperator spOperator;
        if( it->second(spOperator) != sCtx.success() ) {
            return sCtx.error("创建计算器失败");
        }

        if( spOperator->solve(pData, nInVars, pInVars, spOutVar) != sCtx.success() ) {
            return sCtx.error("计算错误");
        }

        return CNnVariableSolver::registerSolvedOperator(spOperator, nInVars, pInVars, spOutVar);
    }
    return sCtx.error();
}

int CNnOperator::solveConv(const char* szPadding, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    PNnConv convData;
    convData.szPadding = szPadding;
    return solveOp("conv", CData<PNnConv>(convData), nInVars, pInVars, spOutVar);
}

int CNnOperator::solvePool(const char* szPadding, int nWidth, int nHeight, int nStride, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    PNnPool poolData;
    poolData.nWidth = nWidth;
    poolData.nHeight = nHeight;
    poolData.nStrideWidth = nStride;
    poolData.nStrideHeight = nStride;
    return solveOp("pool", CData<PNnPool>(poolData), nInVars, pInVars, spOutVar);
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
