#ifndef __SimpleWork_NN_Operators_CReLUOperator_h__
#define __SimpleWork_NN_Operators_CReLUOperator_h__

#include "operator.h"
static SCtx sCtx("ReluOperator");
class CReLUOperator : public CNnSolver, public INnAtomSolver, public IArchivable{
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnSolver)
        SIMPLEWORK_INTERFACE_ENTRY(INnAtomSolver)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnSolver)
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size==outVar.size)
        ((CActivator*)pParameters)->activate(outVar.size, inVars[0].data, outVar.data);
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size==outVar.size)
        ((CActivator*)pParameters)->deactivate(outVar.size, inVars[0].data, outVar.devia, inVars[0].devia);
    }

    int prepareSolver(unsigned int idType, PSolveParameter& solveParameter) {
        solveParameter.pParameter = CActivator::getActivation(idType, "relu");
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
    const char* getClassName() { return "ReLUSolver"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar) {
        return sCtx.success();
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.ReLUSolver"; }

};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CReLUOperator, CReLUOperator::__getClassKey())
static SNnSolverRegister s_Register("relu", CNnSolver::createStaticSolver<CReLUOperator>);

#endif//__SimpleWork_NN_Operators_CReLUOperator_h__