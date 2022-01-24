
#include "nn.h"
#include "CNnWeightVariable.h"
#include "CNnStateVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "CNnInputVariable.h"

#include <map>

static struct PRunCtx {
    PSolveContext* pSolveCtx;
    map<INnInternalVariable*, int> arrSolvedVars;

    int registerVar(const SNnVariable& spVar) {
        int iVar = -1;
        SNnInternalVariable spInternalVar = spVar;
        map<INnInternalVariable*, int>::iterator it = arrSolvedVars.find(spInternalVar.getPtr());
        if(it == arrSolvedVars.end()) {
            iVar = pSolveCtx->arrVars.size();
            arrSolvedVars[spInternalVar.getPtr()] = iVar;
            pSolveCtx->arrVars.push_back(spInternalVar);
        }else{
            iVar = it->second;
        }
        return iVar;
    }
}*s_pRunCtx = nullptr;

static SCtx sCtx("CNnVariableSolver");
int CNnVariableSolver::createWeightVariable(const SDimension& spDimension, SNnVariable& spVar) {
    return CNnWeightVariable::createWeightVariable(spDimension, spVar);
}

int CNnVariableSolver::createStateVariable(const SDimension& spDimension, SNnVariable& spVar){
    return CNnStateVariable::createStateVariable(spDimension, spVar);
}

int CNnVariableSolver::solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CNnOperator::solveOp(szOp, nInVars, pInVars, spOutVar);
}

int CNnVariableSolver::returnSolvedVar(const SNnOperator& spOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    if(s_pRunCtx) {
        SNnVariable spOut;
        if( spOp->getOutVar(spOut) != sCtx.success() ) {
            return sCtx.error("无法获取计算返回值");
        }

        PSolveContext* pCtx = s_pRunCtx->pSolveCtx;
        PSolveContext::PSolveOperator solveParameter;
        solveParameter.nInVars = nInVars;
        int nPushback = 0;
        for( int i=0; i<nInVars; i++) {
            solveParameter.pInVarIndexs[i] = s_pRunCtx->registerVar(pInVars[i]);
        }
        solveParameter.iOutVar = s_pRunCtx->registerVar(spOut);
        solveParameter.spOperator = spOp;

        pCtx->arrOperators.push_back(solveParameter);
        spOutVar = spOut;
        return sCtx.success();
    }

    return spOp->getOutVar(spOutVar);
}

//
// 求解单元函数，虽然做了避免重入处理，但仍然不可以多线程操作
//
int CNnVariableSolver::solveUnit(const SDimension& spInDimension, const SNnUnit& spUnit, PSolveContext* pCtx) {
    if(s_pRunCtx != nullptr) {
        return sCtx.error("求解单元函数，不允许重入");
    }

    PRunCtx runCtx = {
        pCtx
    };
    s_pRunCtx = &runCtx;
    CTaker<PRunCtx*> spContextTaker(&runCtx, [](PRunCtx* pCtx){
        if(s_pRunCtx == pCtx) {
            s_pRunCtx = nullptr;
        }
    });

    //
    // 创建输入变量
    //
    SNnVariable spInput;
    if( CNnInputVariable::createVariable(spInDimension, spInput) != sCtx.success()) {
        return sCtx.error("创建输入变量失败");
    }

    //
    // 求解网络单元，生成网络计算图
    //
    SNnVariable spOutVariable;
    if(spUnit->eval(1, &spInput, spOutVariable) != sCtx.success()) {
        return sCtx.error("网络单元求解失败");
    }

    SNnInternalVariable spOutVar = spOutVariable;
    if( !spOutVar ) {
        return sCtx.error("网络单元求解结果无效");
    }

    pCtx->spInVar = spInput;
    pCtx->spOutVar = spOutVar;
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())