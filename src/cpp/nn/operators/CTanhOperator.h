#ifndef __SimpleWork_NN_Operators_CTanhOperator_h__
#define __SimpleWork_NN_Operators_CTanhOperator_h__

#include "../CNnOperator.h"

class CTanhOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CTanhOperator* pThis = (CTanhOperator*)pParameters;
        pThis->m_pActivator->activate(inVars[0].size, inVars[0].data, outVar.data);
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CTanhOperator* pThis = (CTanhOperator*)pParameters;
        pThis->m_pActivator->deactivate(inVars[0].size, outVar.data, outVar.devia, inVars[0].devia);
    }

    int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) {
        m_pActivator = CActivator::getActivation(idType, "tanh");
        if(idType == CBasicData<float>::getStaticType() ) {
            pEval = evalT<float>;
            pDevia = deviaT<float>;
        }else if(idType == CBasicData<double>::getStaticType() ) {
            pEval = evalT<double>;
            pDevia = deviaT<double>;
        }
        return sCtx.error("类型错误");
    }

    static int createOperator(int nInVars, const SNnVariable pInVars[], SNnOperator& spOutVar) {
        CPointer<CTanhOperator> spOut;
        CObject::createObject(spOut);
        if( int retcode = spOut->initOneEleWiseOperator(nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }

private:
    CActivator* m_pActivator;
};

#endif//__SimpleWork_NN_Operators_CTanhOperator_h__