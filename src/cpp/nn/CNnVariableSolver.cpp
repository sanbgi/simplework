
#include "nn.h"
#include "CNnVariableSolver.h"
#include "CNnSolver.h"

#include <map>

static SCtx sCtx("CNnVariableSolver");

static struct PRunCtx {
    PNnSolveGraph* pSolveCtx;
    map<INnVariable*, int> mapSolvedVars;
    int registerVar(const SNnVariable& spVar) {
        int iVar = -1;
        map<INnVariable*, int>::iterator it = mapSolvedVars.find(spVar.getPtr());
        if(it == mapSolvedVars.end()) {
            iVar = (int)pSolveCtx->arrVars.size();
            mapSolvedVars[spVar.getPtr()] = iVar;
            pSolveCtx->arrVars.push_back(spVar);
        }else{
            iVar = it->second;
        }
        return iVar;
    }
}*s_pRunCtx = nullptr;

int CNnVariableSolver::loadState(const SNnState spState, SNnVariable& spVar) {
    SNnVariable spIn = spState;
    return solveOp("loadState", nullptr, 1, &spIn, spVar);
}

int CNnVariableSolver::saveState(const SNnState spState, const SNnVariable& spVar) {
    SNnVariable o;
    SNnVariable pIn[2] = { spState, spVar };
    return solveOp("saveState", nullptr, 2, pIn, o);
}

int CNnVariableSolver::solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
    return CNnSolver::solveOp(szOp, pData, nInVars, pInVars, spOutVar);
}

int CNnVariableSolver::addAtomOperator(const SNnAtomOperator& spOp, int nInVars, const SNnVariable pInVars[], const SNnVariable& spOutVar) {
    if(s_pRunCtx) {
        PNnSolveGraph* pCtx = s_pRunCtx->pSolveCtx;
        PNnOperator solveParameter;
        solveParameter.nInVars = nInVars;
        for( int i=0; i<nInVars; i++) {
            solveParameter.pInVars[i] = s_pRunCtx->registerVar(pInVars[i]);
        }

        //
        // 如果有输出对象，则注册输出对象
        //
        if(spOutVar) {
            solveParameter.iOutVar = s_pRunCtx->registerVar(spOutVar);
        }else{
            solveParameter.iOutVar = -1;
        }
        pCtx->arrOperatorArgs.push_back(solveParameter);
        pCtx->arrOperators.push_back(spOp);
    }
    return sCtx.success();
}

//
// 求解单元函数，虽然做了避免重入处理，但仍然不可以多线程操作
//
int CNnVariableSolver::solveNetwork(const PNnNetwork* pNet, PNnSolveGraph* pCtx) {
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
    SNnVariable spInput = SObject::createObject("sw.nn.Input", CData<SDimension>(pNet->spInDimension));
    if( !spInput ){
        return sCtx.error("创建输入变量失败");
    }

    //
    // 求解网络单元，生成网络计算图
    //
    SNnVariable spOutVariable;
    if(pNet->pSolver->solve(spInput, spOutVariable) != sCtx.success()) {
        return sCtx.error("网络单元求解失败");
    }

    pCtx->iInVar = runCtx.registerVar(spInput);
    pCtx->iOutVar = runCtx.registerVar(spOutVariable);
    return sCtx.success();
}

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CNnVariableSolver, SNnVariableSolver::__getClassKey())