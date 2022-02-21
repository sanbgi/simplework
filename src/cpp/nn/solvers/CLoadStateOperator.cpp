#ifndef __SimpleWork_NN_Operators_CLoadStateOperator_h__
#define __SimpleWork_NN_Operators_CLoadStateOperator_h__

#include "operator.h"
static SCtx sCtx("LoadStateOperator");

//
// 将状态值拷贝到变量中
//  inVar[0] : 普通变量
//  inVar[1] : 状态变量
//
class CLoadStateOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size == outVar.size)
        Q* pIn = (Q*)inVars[0].data;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pOut = (Q*)outVar.data;
        while(pIn < pInEnd) {
            *pOut = *pIn;
            pIn++, pOut++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size == outVar.size)
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pInDeviaEnd = pInDevia + inVars[0].size;
        Q* pOutDevia = (Q*)outVar.devia;
        while(pInDevia < pInDeviaEnd) {
            *pInDevia += *pOutDevia;
            pInDevia++, pOutDevia++;
        }
    }

    int prepareSolver(unsigned int idType, PSolveParameter& solveParameter) {
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }
    
    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        if(nInVars != 1) {
            return sCtx.error("存储状态参数必须为两个，一个为状态变量，一个为计算变量");
        }
        createVariable(pInVars[0].dimension(), spVarOut);
        return addAtomSolver(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "LoadStateSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.LoadStateSolver"; }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CLoadStateOperator, CLoadStateOperator::__getClassKey())
static SNnSolverRegister s_Register("loadState", CNnSolver::createStaticSolver<CLoadStateOperator>);

#endif//__SimpleWork_NN_Operators_CLoadStateOperator_h__