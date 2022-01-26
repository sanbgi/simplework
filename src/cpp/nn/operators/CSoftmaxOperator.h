#ifndef __SimpleWork_NN_Operators_CSoftmaxOperator_h__
#define __SimpleWork_NN_Operators_CSoftmaxOperator_h__

#include "../CNnOperator.h"

class CSoftmaxOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        pThis->activate(inVars[0].size, inVars[0].data, outVar.data);
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        pThis->deactivate(inVars[0].size, outVar.data, outVar.devia, inVars[0].devia);
    }

    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
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

    int solve(int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        return solveOneEleWise(nInVars, pInVars, spVarOut);
    }
};

#endif//__SimpleWork_NN_Operators_CSoftmaxOperator_h__