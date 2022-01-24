
#include "nn.h"
#include "CNnWeightVariable.h"
#include "CNnStateVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "CNnInputVariable.h"
#include "CNnOperator.h"

#include <map>

static struct PRunCtx {
    PSolveContext* pSolveCtx;
    map<INnInternalVariable*, int> mapSolvedVars;
    map<INnInternalVariable*, SNnInternalVariable> mapReplacement;

    int registerVar(const SNnVariable& spVar, bool bReplacement = true) {
        int iVar = -1;
        SNnInternalVariable spInternalVar = spVar;
        if(bReplacement) {
            map<INnInternalVariable*, SNnInternalVariable>::iterator itReplacement = mapReplacement.find(spInternalVar.getPtr());
            if(itReplacement != mapReplacement.end()) {
                spInternalVar = itReplacement->second;
            }
        }

        map<INnInternalVariable*, int>::iterator it = mapSolvedVars.find(spInternalVar.getPtr());
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
        if( spOp->createOutVar(spOut) != sCtx.success() ) {
            return sCtx.error("无法获取计算返回值");
        }

        PSolveContext* pCtx = s_pRunCtx->pSolveCtx;
        PSolveContext::PSolveOperator solveParameter;
        solveParameter.nInVars = nInVars;
        for( int i=0; i<nInVars; i++) {
            solveParameter.pInVarIndexs[i] = s_pRunCtx->registerVar(pInVars[i]);
        }
        solveParameter.iOutVar = s_pRunCtx->registerVar(spOut);
        solveParameter.spOperator = spOp;
        pCtx->arrOperators.push_back(solveParameter);

        //
        //  输出的时候注意，如果输出目标是state对象，则相当于是两步：
        //      1，将输出数据拷贝到state
        //      2，添加对state的替换，以后使用state，相当于使用当前的output
        //  
        //  为什么state变量需要使用替换机制（即后续对state的引用替换为当前计算结果）？目
        //  的是为了正确求解梯度。由于state值不断在变，但是计算梯度时，是需要当时的state
        //  值的，所以，需要把state替换为计算变量，才能够正确传递梯度；
        // 
        SNnInternalVariable spExternalOut = spOutVar;
        if( spExternalOut && spExternalOut->getVariableType() == ENnVariableType::EVState ) {
            SNnInternalVariable spInternalOut = spOut;
            s_pRunCtx->mapReplacement[spExternalOut.getPtr()] = spInternalOut;
            
            SNnOperator spCopyOp;
            if( CNnOperator::createOp("copy", 1, &spOut, spCopyOp) != sCtx.success() ) {
                return sCtx.error("创建拷贝操作失败");
            }
            
            PSolveContext::PSolveOperator replaceParameter;
            replaceParameter.nInVars = 1;
            replaceParameter.pInVarIndexs[0] = solveParameter.iOutVar;
            replaceParameter.iOutVar = s_pRunCtx->registerVar(spOutVar, false);
            replaceParameter.spOperator = spCopyOp;
            pCtx->arrOperators.push_back(replaceParameter);
        }else{
            spOutVar = spOut;
        }
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

    pCtx->spInVar = spInput;
    pCtx->spOutVar = spOutVar;
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())