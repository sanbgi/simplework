
#include "nn.h"
#include "CNnWeightVariable.h"
#include "CNnStateVariable.h"
#include "CNnOperatorVariable.h"
#include "CNnVariableSolver.h"
#include "CNnInputVariable.h"

#include <map>

static SCtx sCtx("CNnVariableSolver");
int CNnVariableSolver::createWeightVariable(const SDimension& spDimension, SNnVariable& spVar) {
    return CNnWeightVariable::createWeightVariable(spDimension, spVar);
}

int CNnVariableSolver::createStateVariable(const SDimension& spDimension, SNnVariable& spVar){
    return CNnStateVariable::createStateVariable(spDimension, spVar);
}

int CNnVariableSolver::solveOp(const char* szOp, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CNnOperator::createOperator(szOp, nInVars, pInVars, spOutVar);
}

int CNnVariableSolver::solveUnit(const SDimension& spInDimension, const SNnUnit& spUnit, PSolveContext* pCtx) {
    
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

    //
    // 深度遍历网络计算图，生成计算需要的变量及步骤
    //
    map<INnInternalVariable*, int> arrSolvedVars;

    //
    // 输入变量首先作为第一个变量
    //
    SNnInternalVariable spInternalInput = spInput;
    arrSolvedVars[spInternalInput.getPtr()] = pCtx->arrVars.size();
    pCtx->arrVars.push_back(spInput);

    //从求解结果开始遍历
    vector<SNnInternalVariable> arrToSolveVars;
    arrToSolveVars.push_back(spOutVar);
    while(arrToSolveVars.size() > 0) {
        SNnInternalVariable spToSolveVar = arrToSolveVars.at(arrToSolveVars.size()-1);

        //
        // 已经解算的变量，不用重复解算
        //
        if( arrSolvedVars.find(spToSolveVar.getPtr()) != arrSolvedVars.end()) {
            arrToSolveVars.pop_back();
            continue;
        }

        //
        // 分类型处理未解算变量
        //
        switch(spToSolveVar->getVariableType()) {
            case ENnVariableType::EVInput:
            case ENnVariableType::EVState:
            case ENnVariableType::EVWeight:
            {
                arrSolvedVars[spToSolveVar.getPtr()] = pCtx->arrVars.size();
                pCtx->arrVars.push_back(spToSolveVar);
                arrToSolveVars.pop_back();
            }
            break;

            case ENnVariableType::EVOperator:
            {
                SNnInternalVariable pSubVars[4];
                int nSubVars = spToSolveVar->getSubVariables(pSubVars);

                PSolveContext::PSolveOperator solveParameter;
                solveParameter.nInVars = nSubVars;
                int nPushback = 0;
                for( int i=0; i<nSubVars; i++) {
                    SNnInternalVariable spSubVar = pSubVars[i];
                    map<INnInternalVariable*, int>::iterator it = arrSolvedVars.find(spSubVar.getPtr());
                    if(it == arrSolvedVars.end()) {
                        arrToSolveVars.push_back(spSubVar);
                        nPushback ++;
                    }else{
                        solveParameter.pInVarIndexs[i] = it->second;
                    }
                }

                if(nPushback == 0) {
                    arrSolvedVars[spToSolveVar.getPtr()] = pCtx->arrVars.size();

                    solveParameter.iOutVar = pCtx->arrVars.size();
                    solveParameter.spOperator = ((CNnOperatorVariable*)spToSolveVar->getVariablePtr())->m_spOperator;

                    pCtx->arrOperators.push_back(solveParameter);
                    pCtx->arrVars.push_back(spToSolveVar);
                    arrToSolveVars.pop_back();
                }
            }
            break;

            default:{
                return sCtx.error("未知错误，发现不认识的结算变量类型");
            }
            break;
        }
    }
    pCtx->spInVar = spInput;
    pCtx->spOutVar = spOutVar;
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())