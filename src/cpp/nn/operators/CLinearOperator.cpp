#ifndef __SimpleWork_NN_Operators_CLinearOperator_h__
#define __SimpleWork_NN_Operators_CLinearOperator_h__

#include "operator.h"
static SCtx sCtx("LinearOperator");
class CLinearOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
        Q* pIn = (Q*)inVars[0].data;
        Q* pMat = (Q*)inVars[1].data;
        Q* pOut = (Q*)outVar.data;
        Q* pItIn, *pItOut, *pItMat;
        Q* pItOutEnd, *pItInEnd;
        Q sumV;

        int nIn = pThis->m_nInputSize;
        int nOut = pThis->m_nOutputSize;
        int nTensor = pThis->m_nTensor;
        while(nTensor-->0) {
            pItOut = pOut;
            pItMat = pMat;
            pItOutEnd = pItOut+nOut;
            while(pItOut<pItOutEnd) {
                sumV = 0;
                pItIn = pIn;
                pItInEnd = pIn+nIn;
                while(pItIn<pItInEnd) {
                    sumV = *pItIn * *pItMat;
                    pIn++, pMat++;
                }
                *pOut = sumV;
                pOut++;
            }
            pIn += nIn;
            pOut += nOut;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        VERIFY(nInVars==2)
        CLinearOperator* pThis = (CLinearOperator*)pParameters;
        Q* pIn = (Q*)inVars[0].data;
        Q* pInDevia = (Q*)inVars[0].devia;
        Q* pMat = (Q*)inVars[1].data;
        Q* pMatDevia = (Q*)inVars[1].devia;
        Q* pOutDevia = (Q*)outVar.devia;
        Q *pItIn, *pItInDevia, *pItMat, *pItMatDevia, *pItOutDevia;
        Q *pItOutDeviaEnd, *pItInEnd;
        Q deviaV;
        int nIn = pThis->m_nInputSize;
        int nOut = pThis->m_nOutputSize;
        int nTensor = pThis->m_nTensor;
        while(nTensor-->0) {
            pItOutDevia = pOutDevia;
            pItMat = pMat;
            pItMatDevia = pMatDevia;
            pItOutDeviaEnd = pItOutDevia+nOut;
            while(pItOutDevia<pItOutDeviaEnd) {
                deviaV = *pItOutDevia;
                pItIn = pIn;
                pItInDevia = pInDevia;
                pItInEnd = pIn+nIn;
                while(pItIn<pItInEnd) {
                    *pItInDevia += *pMat * deviaV;
                    *pItMatDevia += *pIn * deviaV;
                    pIn++, pInDevia++, pMat++, pMatDevia++;
                }
                pItOutDevia++;
            }
            pIn += nIn;
            pInDevia += nIn;
            pOutDevia += nOut;
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

        m_nTensor = spDim1.dataSize()/nInputSize;
        m_nInputSize = nInputSize;
        m_nOutputSize = nOutputSize;
        SDimension spOutDim = spDim1.downLowDimension().upLowDimension(nOutputSize);
        return createVariable(spOutDim, spVarOut);
    }

private:
    //从多长的向量线性变化成另外一个长度向量，都是在最低维度，高纬度保留
    int m_nTensor;
    int m_nInputSize;
    int m_nOutputSize;
};

static SNnOperatorRegister s_Register("Linear", CNnOperator::createOperator<CLinearOperator>);

#endif//__SimpleWork_NN_Operators_CLinearOperator_h__