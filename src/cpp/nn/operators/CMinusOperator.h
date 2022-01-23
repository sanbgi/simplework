#ifndef __SimpleWork_NN_Operators_CMinusOperator_h__
#define __SimpleWork_NN_Operators_CMinusOperator_h__

#include "../CNnOperator.h"

class CMinusOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn2 = (Q*)inVars[1].data;
        Q* pO = (Q*)outVar.data;
        Q* pOEnd = pO + outVar.size;
        while(pO < pOEnd) {
            *pO = *pIn1 - *pIn2;
            pO++, pIn1++, pIn2++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDeviaO = (Q*)outVar.devia;
        Q* pDeviaOEnd = pDeviaO + outVar.size;
        while(pDeviaO < pDeviaOEnd) {
            *pDevia1 += *pDeviaO;
            *pDevia2 += -*pDeviaO;
            pDevia1++, pDevia2++, pDeviaO++;
        }
    }

    int getEvalFunAddress(unsigned int idType, FEval& pEval, FEval& pDevia) {
        if(idType == CBasicData<float>::getStaticType() ) {
            pEval = evalT<float>;
            pDevia = deviaT<float>;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            pEval = evalT<double>;
            pDevia = deviaT<double>;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    static int createOperator(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
        CPointer<CMinusOperator> spOut;
        CObject::createObject(spOut);
        if( int retcode = spOut->initTwoEleWiseOperator(nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }
};

#endif//__SimpleWork_NN_Operators_CMinusOperator_h__