#ifndef __SimpleWork_NN_Operators_CLinearOperator_h__
#define __SimpleWork_NN_Operators_CLinearOperator_h__

#include "operator.h"
static SCtx sCtx("LinearOperator");
class CLinearOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        VERIFY(nInVars==2)
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
        int nIn = pThis->nIn;
        int nMat = pThis->nMat;
        int nOut = pThis->nOut;
        Q* pIn = (Q*)inVars[0].data;
        Q* pMat = (Q*)inVars[1].data;
        Q* pOut = (Q*)outVar.data;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pMatEnd = pMat + inVars[1].size;
        Q* pItIn, *pItMat;
        Q* pItInEnd, *pItOutEnd;
        Q v;
        while(nBatchs-->0) {
            pItMat = pMat;
            pItOutEnd = pOut + nOut;
            while(pOut != pItOutEnd) {
                v = 0;
                pItIn = pIn;
                pItInEnd = pIn + nIn;
                while(pItIn < pItInEnd) {
                    v += (*pItIn) * (*pItMat);
                    pItIn++, pItMat++;
                }
                *pOut = v;
                pOut++;
            }
            pIn += nIn;
            pMat += nMat;
            if(pIn == pInEnd) {
                pIn = (Q*)inVars[0].data;
            }
            if(pMat == pMatEnd) {
                pMat = (Q*)inVars[1].data;
            }
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
        int nIn = pThis->nIn;
        int nMat = pThis->nMat;
        int nOut = pThis->nOut;
        Q* pIn = (Q*)inVars[0].data;
        Q* pInDeiva = (Q*)inVars[0].devia;
        Q* pMat = (Q*)inVars[1].data;
        Q* pMatDevia = (Q*)inVars[1].devia;
        Q* pOutDevia = (Q*)outVar.devia;
        Q* pInEnd = pIn + inVars[0].size;
        Q* pMatEnd = pMat + inVars[1].size;
        Q* pItInEnd, *pOutDeviaEnd;
        Q* pItIn, *pItInDevia;
        Q deviationOut;
        Q* pItMat;
        Q* pItMatDevia;
        while(nBatchs-->0) {
            pItMat = pMat;
            pItMatDevia = pMatDevia;
            pOutDeviaEnd = pOutDevia + nOut;
            while(pOutDevia != pOutDeviaEnd) {
                pItIn = pIn;
                pItInDevia = pInDeiva;
                deviationOut = *pOutDevia;
                pItInEnd = pIn + nIn;
                while(pItIn < pItInEnd) {
                    *pItInDevia += deviationOut * (*pItMat);
                    *pItMatDevia += deviationOut * (*pItIn);
                    pItIn++, pItInDevia++, pItMat++, pItMatDevia++;
                }
                pOutDevia++;
            }
            pIn += nIn;
            pInDeiva += nIn;
            pMat += nMat;
            pMatDevia += nMat;
            if(pIn == pInEnd) {
                pIn = (Q*)inVars[0].data;
                pInDeiva = (Q*)inVars[0].devia;
            }
            if(pMat == pMatEnd) {
                pMat = (Q*)inVars[1].data;
                pMatDevia = (Q*)inVars[1].devia;
            }
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

    int solve(const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spVarOut) {
        SDimension spDim1 = pInVars[0].dimension();
        SDimension spDim2 = pInVars[1].dimension();
        if(spDim2.size() != 2 ) {
            return sCtx.error("线性变换第二个参数必须是变换矩阵");
        }

        int nDim1 = spDim1.size();
        int nDim2 = spDim2.size();
        if(nDim1 < 1) {
            return sCtx.error("线性变换参数，维度不能为零");
        }

        int nInputSize = spDim1.data()[nDim1-1];
        int nOutputSize = spDim2.data()[0];
        if( spDim1.data()[nDim1-1] != spDim2.data()[1] ) {
            return sCtx.error("线性变换的两个张量尺寸不匹配，第一个张量的最低维度必须与第二个张量的最低维度一致");
        }

        nIn = nInputSize;
        nOut = nOutputSize;
        nMat = nIn * nOut;
        SDimension spOutDim = spDim1.downLowDimension().upLowDimension(nOutputSize);
        return createVariable(spOutDim, spVarOut);
    }

private:
    //从多长的向量线性变化成另外一个长度向量，都是在最低维度，高纬度保留
    int nIn;
    int nMat;
    int nOut;
};

static SNnOperatorRegister s_Register("linear", CNnOperator::createOperator<CLinearOperator>);

#endif//__SimpleWork_NN_Operators_CLinearOperator_h__