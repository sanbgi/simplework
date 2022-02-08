#ifndef __SimpleWork_NN_Operators_CTanhOperator_h__
#define __SimpleWork_NN_Operators_CTanhOperator_h__

#include "operator.h"
static SCtx sCtx("TanhOperator");
class CTanhOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        pThis->activate(inVars[0].size, inVars[0].data, outVar.data);
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CActivator* pThis = (CActivator*)pParameters;
        pThis->deactivate(inVars[0].size, inVars[0].data, outVar.devia, inVars[0].devia);
    }

    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
        solveParameter.pParameter = CActivator::getActivation(idType, "tanh");
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
        return solveOneEleWise(nInVars, pInVars, spVarOut);
    }
};

static SNnOperatorRegister s_Register("tanh", CNnOperator::createStaticOperator<CTanhOperator>);

#endif//__SimpleWork_NN_Operators_CTanhOperator_h__