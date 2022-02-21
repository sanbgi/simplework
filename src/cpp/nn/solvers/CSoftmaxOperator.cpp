#ifndef __SimpleWork_NN_Operators_CSoftmaxOperator_h__
#define __SimpleWork_NN_Operators_CSoftmaxOperator_h__

#include "operator.h"
static SCtx sCtx("SoftmaxOperator");
class CSoftmaxOperator : public CNnSolver, public INnAtomOperator, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomOperator)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        int nBatchSize = outVar.size / nBatchs;
        Q* pIn = (Q*)inVars[0].data;
        Q* pOut = (Q*)outVar.data;
        while(nBatchs-->0) {
            pThis->activate(nBatchSize, pIn, pOut);
            pIn += nBatchSize;
            pOut += nBatchSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        int nBatchSize = outVar.size / nBatchs;
        Q* pIn = (Q*)inVars[0].data;
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pOutDevia = (Q*)outVar.devia;
        while(nBatchs-->0) {
            pThis->deactivate(nBatchSize, pIn, pOutDevia, pInDevia);
            pIn += nBatchSize;
            pInDevia += nBatchSize;
            pOutDevia += nBatchSize;
        }
    }

    int prepareSolver(unsigned int idType, PSolveParameter& solveParameter) {
        solveParameter.pParameter = CActivator::getActivation(idType, "softmax");
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveOneEleWise(this, nInVars, pInVars, spVarOut);
    }

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "SoftmaxSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.SoftmaxSolver"; }

};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CSoftmaxOperator, CSoftmaxOperator::__getClassKey())
static SNnSolverRegister s_Register("softmax", CNnSolver::createStaticSolver<CSoftmaxOperator>);

#endif//__SimpleWork_NN_Operators_CSoftmaxOperator_h__