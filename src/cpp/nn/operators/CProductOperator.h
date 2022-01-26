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

    int getSolveParameter(unsigned int idType, PSolveParameter& solveParameter) {
        if(idType == CBasicData<float>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<float>;
            solveParameter.pDeviaFun = deviaT<float>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }else if(idType == CBasicData<double>::getStaticType() ) {
            solveParameter.pEvalFun = evalT<double>;
            solveParameter.pDeviaFun = deviaT<double>;
            solveParameter.pParameter = this;
            return sCtx.success();
        }
        return sCtx.error("类型错误");
    }

    int solve(int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
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

        return createVariable(SDimension(1,pDimSize2), spVarOut);
    }
};

#endif//__SimpleWork_NN_Operators_CProductOperator_h__