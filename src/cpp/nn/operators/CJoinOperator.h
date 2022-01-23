#ifndef __SimpleWork_NN_Operators_CJoinOperator_h__
#define __SimpleWork_NN_Operators_CJoinOperator_h__

#include "../CNnOperator.h"

class CJoinOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pIn1 = (Q*)inVars[0].data;
        Q* pIn1End = pIn1 + inVars[0].size;

        Q* pIn2 = (Q*)inVars[1].data;
        Q* pIn2End = pIn2 + inVars[1].size;

        Q* pO = (Q*)outVar.data;
        while(pIn1 < pIn1End) {
            *pO = *pIn1;
            pO++, pIn1++;
        }
        while(pIn2 < pIn1End) {
            *pO = *pIn2;
            pO++, pIn2++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pDevia1 = (Q*)inVars[0].devia;
        Q* pDevia1End = pDevia1 + inVars[0].size;

        Q* pDevia2 = (Q*)inVars[1].devia;
        Q* pDevia2End = pDevia2 + inVars[1].size;

        Q* pDeviaO = (Q*)outVar.devia;
        while(pDevia1 < pDevia1End) {
            *pDevia1 += *pDeviaO;
            pDevia1++, pDeviaO++;
        }
        while(pDevia2 < pDevia1End) {
            *pDevia2 += *pDeviaO;
            pDevia2++, pDeviaO++;
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

        if(nInVars != 2) {
            return sCtx.error("连接需要两个参数");
        }

        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        if( spDim1.size() != 1 || spDim2.size() != 1) {
            return sCtx.error("连接只适用于连接两个向量");
        }

        CPointer<CJoinOperator> spOut;
        CObject::createObject(spOut);
        if( int retcode = spOut->initOperator(nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }

        int size1 = *spDim1.data();
        int size2 = *spDim2.data();
        int size = size1+size2;
        if( SDimension::createDimension(spOut->m_spDimension,1,&size) != sCtx.success() ) {
            return sCtx.error();
        }

        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }
};

#endif//__SimpleWork_NN_Operators_CJoinOperator_h__