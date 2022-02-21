#include "CNnSolver.h"
#include "CUtils.h"
#include "CActivator.h"
#include "CNnVariableSolver.h"
#include "variables/CNnOperatorVariable.h"
#include <map>

static SCtx sCtx("CNnSolver");
map<string, FCreateSolver>& CNnSolver::getFactories() {
    static map<string, FCreateSolver> s_opFactories;
    return s_opFactories;
}

int CNnSolver::regisetOperator(const char* szOperator, FCreateSolver funCreator) {
    getFactories()[szOperator] = funCreator;
    return sCtx.success();
}

int CNnSolver::solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    map<string, FCreateSolver>::iterator it = getFactories().find(szOp);
    if(it != getFactories().end()) {
        SNnSolver spSolver;
        if( it->second(spSolver) != sCtx.success() ) {
            return sCtx.error("创建计算器失败");
        }
        return spSolver->solve(pData, nInVars, pInVars, spOutVar);
    }
    return sCtx.error();
}

int CNnSolver::addAtomSolver(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar){
    return CNnVariableSolver::addAtomSolver(pSolver, nInVars, pInVars, spOutVar);
}

int CNnSolver::solveOneEleWise(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    if(nInVars != 1) {
        return sCtx.error("参数个数不等于二");
    }

    SDimension spDimension = pInVars[0].dimension();
    CNnOperatorVariable::createOperatorVariable(spDimension, spOutVar);
    return addAtomSolver(pSolver, nInVars, pInVars, spOutVar);
}

int CNnSolver::solveTwoEleWise(INnAtomOperator* pSolver, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar){
    if(nInVars != 2) {
        return sCtx.error("参数个数不等于二");
    }

    //
    // 两个张量进行元素级别的操作时，必须符合下面两种情况之一
    //  1，两个张量维度完全相同
    //  2，两个张量的最低维度尺寸相同，并且第二个张量维度为一维
    //
    SDimension spInDimension1 = pInVars[0].dimension();
    SDimension spInDimension2 = pInVars[1].dimension();
    if(spInDimension1.size() > spInDimension2.size()) {
        int size1 = spInDimension1.size();
        int size2 = spInDimension2.size();
        if( size2 > 1) {
            return sCtx.error("当两个张量维度不同时，不支持第二个张量维度大于一维");
        }

        if( spInDimension1.data()[size1-1] != spInDimension2.data()[size2-1]) {
            return sCtx.error("两个张量不能进行元素运算，因为维度不一致");
        }
    }else{
        if( !spInDimension1.isEqual(spInDimension2) ) {
            return sCtx.error("相加的两个元素维度不一致");
        }
    }
    CNnOperatorVariable::createOperatorVariable(spInDimension1, spOutVar);
    return addAtomSolver(pSolver, nInVars, pInVars, spOutVar);
}

int CNnSolver::createVariable(const SDimension& spDimension, SNnVariable& spOutVar) {
    return CNnOperatorVariable::createOperatorVariable(spDimension, spOutVar);
}
