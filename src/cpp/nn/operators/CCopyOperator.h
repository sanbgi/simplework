#ifndef __SimpleWork_NN_Operators_CCopyOperator_h__
#define __SimpleWork_NN_Operators_CCopyOperator_h__

#include "../CNnOperator.h"

class CCopyOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn = (Q*)inVars[0].data;
        Q* pIn1End = pIn + inVars[0].size;
        Q* pO = (Q*)outVar.data;
        while(pIn < pIn1End) {
            *pO = *pIn;
            pO++, pIn++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pDevia = (Q*)inVars[0].devia;
        Q* pDeviaEnd = pDevia + inVars[0].size;
        Q* pDeviaO = (Q*)outVar.devia;
        while(pDevia < pDeviaEnd) {
            *pDevia += *pDeviaO;
            pDevia++, pDeviaO++;
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

    static int createOperator(int nInVars, const SNnVariable pInVars[], SNnOperator& spOutVar) {
        if(nInVars != 1) {
            return sCtx.error("拷贝只需要一个参数");
        }

        CPointer<CCopyOperator> spOut;
        CObject::createObject(spOut);
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }
};

#endif//__SimpleWork_NN_Operators_CCopyOperator_h__