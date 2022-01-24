#ifndef __SimpleWork_NN_Operators_CProductOperator_h__
#define __SimpleWork_NN_Operators_CProductOperator_h__

#include "../CNnOperator.h"

class CProductOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pV1 = (Q*)inVars[0].data;
        Q* pV2 = (Q*)inVars[1].data;
        Q* pVO = (Q*)outVar.data;
        Q* pVOEnd = pVO + outVar.size;
        Q* pV1End = pV1 + inVars[0].size;
        Q* pItIn;
        Q v;
        while(pVO != pVOEnd) {
            v = 0;
            pItIn = pV1;
            while(pItIn < pV1End) {
                v += (*pItIn) * (*pV2);
                pItIn++, pV2++;
            }
            *pVO = v;
            pVO++;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        Q* pInput1 = (Q*)inVars[0].data;
        Q* pD1 = (Q*)inVars[0].devia;
        Q* pWeights = (Q*)inVars[1].data;
        Q* pWeightDeviations = (Q*)inVars[1].devia;
        Q* pDeviaOut = (Q*)outVar.devia;
        Q* pDeviaOutEnd = pDeviaOut + outVar.size;
        Q* pInput1End = pInput1 + inVars[0].size;
        Q* pIn, *pInDeviation;
        Q deviationOut;
        while(pDeviaOut != pDeviaOutEnd) {
            pIn = pInput1;
            pInDeviation = pD1;
            deviationOut = *pDeviaOut;
            while(pIn < pInput1End) {
                *pInDeviation += deviationOut * (*pWeights);
                *pWeightDeviations += deviationOut * (*pIn);
                pIn++, pInDeviation++, pWeights++, pWeightDeviations++;
            }
            pDeviaOut++;
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
        CPointer<CProductOperator> spOut;
        CObject::createObject(spOut);
        if( int retcode = spOut->initOperator(nInVars, pInVars) != sCtx.success() ) {
            return retcode;
        }

        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        if(spDim2.size() != 2 ) {
            return sCtx.error("点乘只适用于向量乘以矩阵");
        }

        int nInElementSize = spDim1.dataSize();
        const int* pDimSize2 = spDim2.data();
        if(nInElementSize != pDimSize2[1]) {
            return sCtx.error("向量和矩阵点乘的尺寸不匹配");
        }

        if( SDimension::createDimension(spOut->m_spDimension, 1, pDimSize2) != sCtx.success()) {
            return sCtx.error("创建输出张量的维度向量失败");
        }
        spOutVar.setPtr(spOut.getPtr());
        return sCtx.success();
    }
};

#endif//__SimpleWork_NN_Operators_CProductOperator_h__