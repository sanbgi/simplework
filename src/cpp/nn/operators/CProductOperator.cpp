#ifndef __SimpleWork_NN_Operators_CProductOperator_h__
#define __SimpleWork_NN_Operators_CProductOperator_h__

#include "operator.h"
static SCtx sCtx("ProductOperator");
class CProductOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        //VERIFY(inVars[0].size * outVar.size == inVars[1].size)
        Q* pV1 = (Q*)inVars[0].data;
        Q* pV2 = (Q*)inVars[1].data;
        Q* pVO = (Q*)outVar.data;
        int nInSize = inVars[0].size / nBatchs;
        int iOutSize, nOutSize = outVar.size / nBatchs;
        Q* pItIn, *pItMat;
        Q* pV1End, *pVOEnd;
        Q v;
        while(nBatchs-->0) {
            pItMat = pV2;
            pVOEnd = pVO + nOutSize;
            while(pVO != pVOEnd) {
                v = 0;
                pItIn = pV1;
                pV1End = pV1 + nInSize;
                while(pItIn < pV1End) {
                    v += (*pItIn) * (*pItMat);
                    pItIn++, pItMat++;
                }
                *pVO = v;
                pVO++;
            }
            pV1 += nInSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        //VERIFY(inVars[0].size * outVar.size == inVars[1].size)
        Q* pInput = (Q*)inVars[0].data;
        Q* pInputDeiva = (Q*)inVars[0].devia;
        Q* pWeights = (Q*)inVars[1].data;
        Q* pWeightDevia = (Q*)inVars[1].devia;
        Q* pDeviaOut = (Q*)outVar.devia;
        int nInSize = inVars[0].size / nBatchs;
        int nOutSize = outVar.size / nBatchs;

        Q* pInputEnd, *pDeviaOutEnd;
        Q* pItIn, *pItInDevia;
        Q deviationOut;
        Q* pItWeight;
        Q* pItWeightDevia;
        while(nBatchs-->0) {
            pItWeight = pWeights;
            pItWeightDevia = pWeightDevia;
            pDeviaOutEnd = pDeviaOut + nOutSize;
            while(pDeviaOut != pDeviaOutEnd) {
                pItIn = pInput;
                pItInDevia = pInputDeiva;
                deviationOut = *pDeviaOut;
                pInputEnd = pInput + nInSize;
                while(pItIn < pInputEnd) {
                    *pItInDevia += deviationOut * (*pItWeight);
                    *pItWeightDevia += deviationOut * (*pItIn);
                    pItIn++, pItInDevia++, pItWeight++, pItWeightDevia++;
                }
                pDeviaOut++;
            }
            pInput += nInSize;
            pInputDeiva += nInSize;
        }
        //VERIFY(pWeightDevia - (Q*)inVars[1].devia == inVars[1].size )
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

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
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
        isBatchInVars[0] = isBatchInVariable(pInVars[0]);
        isBatchInVars[1] = isBatchInVariable(pInVars[1]);
        return createVariable(SDimension(1,pDimSize2), spVarOut);
    }

private:
    int isBatchInVars[2];
};

static SNnOperatorRegister s_Register("product", CNnOperator::createOperator<CProductOperator>);

#endif//__SimpleWork_NN_Operators_CProductOperator_h__