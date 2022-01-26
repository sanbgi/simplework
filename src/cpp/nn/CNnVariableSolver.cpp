
#include "nn.h"
#include "CNnWeightVariable.h"
#include "CNnStateVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "CNnInputVariable.h"
#include "CNnOperator.h"

#include <map>

static SCtx sCtx("CNnVariableSolver");

static struct PRunCtx {
    PSolveContext* pSolveCtx;
    map<INnVariable*, int> mapSolvedVars;
    int registerVar(const SNnVariable& spVar, bool bReplacement = true) {
        int iVar = -1;
        SNnVariable spInternalVar = spVar;
        map<INnVariable*, int>::iterator it = mapSolvedVars.find(spInternalVar.getPtr());
        if(it == mapSolvedVars.end()) {
            iVar = pSolveCtx->arrVars.size();
            mapSolvedVars[spInternalVar.getPtr()] = iVar;
            pSolveCtx->arrVars.push_back(spInternalVar);
        }else{
            iVar = it->second;
        }
        return iVar;
    }
}*s_pRunCtx = nullptr;

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
        if( spOp->createOutVar(spOut) != sCtx.success() ) {
            return sCtx.error("无法获取计算返回值");
        }

        PSolveContext* pCtx = s_pRunCtx->pSolveCtx;
        PSolveContext::PSolveOperator solveParameter;
        solveParameter.nInVars = nInVars;
        for( int i=0; i<nInVars; i++) {
            solveParameter.pInVars[i] = s_pRunCtx->registerVar(pInVars[i]);
        }

        //
        // 如果有输出对象，则注册输出对象
        //
        if(spOut) {
            solveParameter.iOutVar = s_pRunCtx->registerVar(spOut);
        }else{
            solveParameter.iOutVar = -1;
        }
        solveParameter.spOperator = spOp;
        pCtx->arrOperators.push_back(solveParameter);

        spOutVar = spOut;
        return sCtx.success();
    }

    return spOp->createOutVar(spOutVar);
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

    pCtx->iInVar = runCtx.registerVar(spInput,false);
    pCtx->iOutVar = runCtx.registerVar(spOutVar,false);
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())