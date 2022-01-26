#ifndef __SimpleWork_NN_Operators_CReLUOperator_h__
#define __SimpleWork_NN_Operators_CReLUOperator_h__

#include "../CNnOperator.h"

class CReLUOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size==outVar.size)
        ((CActivator*)pParameters)->activate(inVars[0].size, inVars[0].data, outVar.data);
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==1)
        VERIFY(inVars[0].size==outVar.size)
        ((CActivator*)pParameters)->deactivate(inVars[0].size, outVar.data, outVar.devia, inVars[0].devia);
    }

    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
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

    int solve(int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveOneEleWise(nInVars, pInVars, spVarOut);
    }
};

#endif//__SimpleWork_NN_Operators_CReLUOperator_h__