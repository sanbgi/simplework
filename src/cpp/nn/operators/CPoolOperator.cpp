#ifndef __SimpleWork_NN_Operators_CPoolOperator_h__
#define __SimpleWork_NN_Operators_CPoolOperator_h__

#include "operator.h"

static SCtx sCtx("PoolOperator");
class CPoolOperator : public CNnOperator {
public:
    template<typename Q>
    static void evalT(void* pParameters, int nBatchs, int nInVars, PVector inVars[], PVector outVar) {
        CPoolOperator* pThis = (CPoolOperator*)pParameters;
            
        int nPoolWidth = pThis->m_nPoolWidth;
        int nPoolHeight = pThis->m_nPoolHeight;
        int nOutWidth = pThis->m_nOutWidth;
        int nOutHeight = pThis->m_nOutHeight;
        int nTensor = nBatchs;
        int nLayer = pThis->m_nInputLayer;
        int nInputTensorSize = pThis->m_nInputTensorSize;
        int nOutputTensorSize = pThis->m_nOutTensorSize;

        int nInputHstep = pThis->m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
        int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;

        int nOutHstep = pThis->m_nOutWidth * nLayer;
        int nOutWstep = nLayer;

        struct CItOutVariables {
            Q* pIn;
            Q* pOut;
        }it = {
            (Q*)inVars[0].data,
            (Q*)outVar.data,
        };
        for(int iTensor=0; iTensor<nTensor; iTensor++) {
            CItOutVariables varTBackup = {
                it.pIn,
                it.pOut,
            };
            for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
                CItOutVariables varOYBackup;
                varOYBackup.pIn = it.pIn;
                varOYBackup.pOut = it.pOut;
                for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                    CItOutVariables varOXBackup;
                    varOXBackup.pIn = it.pIn;
                    varOXBackup.pOut = it.pOut;
                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pIn = it.pIn;
                        varConvBackup.pOut = it.pOut;

                        Q dMax = (*it.pIn);

                        //
                        //  从输入中找到最大的那个值
                        //  
                        for( int iConvY=0; iConvY<nPoolHeight; iConvY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pIn = it.pIn;
                            for( int iConvX=0; iConvX<nPoolWidth; iConvX++) {
                                if( (*it.pIn) > dMax) {
                                    dMax = (*it.pIn);
                                }
                                it.pIn += nInputWstep;
                            }
                            it.pIn = varConvYBackup.pIn + nInputHstep;
                        }

                        (*it.pOut) = dMax;

                        it.pIn = varConvBackup.pIn + 1;
                        it.pOut = varConvBackup.pOut + 1;;
                    }
                    it.pIn = varOXBackup.pIn + nInStrideWstep;
                    it.pOut = varOXBackup.pOut + nOutWstep;
                }
                it.pIn = varOYBackup.pIn + nInStrideHstep;
                it.pOut = varOYBackup.pOut + nOutHstep;
            }
            it.pIn = varTBackup.pIn + nInputTensorSize;
            it.pOut = varTBackup.pOut + nOutputTensorSize;
        }
    }

    template<typename Q>
    static void deviaT(void* pParameters, int nBatchs, int nInVars, PDeviaVector inVars[], PDeviaVector outVar) {
        CPoolOperator* pThis = (CPoolOperator*)pParameters;
            
        int nLayer = pThis->m_nInputLayer;
        int nInputWidth = pThis->m_nInputWidth;
        int nInputHeight = pThis->m_nInputHeight;
        int nInputLayer = pThis->m_nInputLayer;
        int nPoolWidth = pThis->m_nPoolWidth;
        int nPoolHeight = pThis->m_nPoolHeight;
        int nOutWidth = pThis->m_nOutWidth;
        int nOutHeight = pThis->m_nOutHeight;

        int nTensor = nBatchs;
        int nInputTensorSize = pThis->m_nInputTensorSize;
        int nOutputTensorSize = pThis->m_nOutTensorSize;

        int nInputHstep = pThis->m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * pThis->m_nStrideHeight;
        int nInStrideWstep = nInputWstep * pThis->m_nStrideWidth;
        
        int nOutHstep = nOutWidth * nLayer;
        int nOutWstep = nLayer;

        struct CItOutVariables {
            Q* pIn;
            Q* pInDeviation;
            Q* pOutDeviation;
        }it = {
            (Q*)inVars[0].data,
            (Q*)inVars[0].devia,
            (Q*)outVar.devia,
        };
        for(int iTensor=0; iTensor<nTensor; iTensor++) {
            CItOutVariables varTBackup = {
                it.pIn,
                it.pInDeviation,
                it.pOutDeviation
            };
            for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
                CItOutVariables varOYBackup;
                varOYBackup.pIn = it.pIn;
                varOYBackup.pInDeviation = it.pInDeviation;
                varOYBackup.pOutDeviation = it.pOutDeviation;
                for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                    CItOutVariables varOXBackup;
                    varOXBackup.pIn = it.pIn;
                    varOXBackup.pInDeviation = it.pInDeviation;
                    varOXBackup.pOutDeviation = it.pOutDeviation;
                    for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                        CItOutVariables varConvBackup;
                        varConvBackup.pIn = it.pIn;
                        varConvBackup.pInDeviation = it.pInDeviation;
                        varConvBackup.pOutDeviation = it.pOutDeviation;

                        Q dMax = (*it.pIn);
                        Q* pExpectDelta = it.pInDeviation;

                        //
                        //  从输入中找到最大的那个点，作为反向传到的点
                        //  
                        for( int iConvY=0; iConvY<nPoolHeight; iConvY++) {
                            CItOutVariables varConvYBackup;
                            varConvYBackup.pIn = it.pIn;
                            varConvYBackup.pInDeviation = it.pInDeviation;
                            for( int iConvX=0; iConvX<nPoolWidth; iConvX++) {
                                if( (*it.pIn) > dMax) {
                                    dMax = (*it.pIn);
                                    pExpectDelta = it.pInDeviation;
                                }
                                it.pIn += nInputWstep;
                                it.pInDeviation += nInputWstep;
                            }
                            it.pIn = varConvYBackup.pIn + nInputHstep;
                            it.pInDeviation = varConvYBackup.pInDeviation + nInputHstep;
                        }

                        (*pExpectDelta) = (*it.pOutDeviation);

                        it.pIn = varConvBackup.pIn + 1;
                        it.pInDeviation = varConvBackup.pInDeviation + 1;
                        it.pOutDeviation = varConvBackup.pOutDeviation + 1;;
                    }

                    it.pIn = varOXBackup.pIn + nInStrideWstep;
                    it.pInDeviation = varOXBackup.pInDeviation + nInStrideWstep;
                    it.pOutDeviation = varOXBackup.pOutDeviation + nOutWstep;
                }

                it.pIn = varOYBackup.pIn + nInStrideHstep;
                it.pInDeviation = varOYBackup.pInDeviation + nInStrideHstep;
                it.pOutDeviation = varOYBackup.pOutDeviation + nOutHstep;
            }

            //  更新迭代参数
            it.pIn = varTBackup.pIn + nInputTensorSize;
            it.pInDeviation = varTBackup.pInDeviation + nInputTensorSize;
            it.pOutDeviation = varTBackup.pOutDeviation + nOutputTensorSize;
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
        if(nInVars != 1) {
            return sCtx.error("卷积操作需要1个输入数据");
        }
        if(pData == nullptr) {
            return sCtx.error("缺少初始化参数");
        }

        const PNnPool* pPool = CData<PNnPool>(*pData);
        if(pPool == nullptr) {
            return sCtx.error("错误的初始化参数");
        }

        m_nPoolWidth = pPool->nWidth;
        m_nPoolHeight = pPool->nHeight;
        m_nStrideWidth = pPool->nStrideWidth;
        m_nStrideHeight = pPool->nStrideHeight;

        SDimension spDim1 = pInVars[0].dimension();
        if(spDim1.dataSize() < 2) {
            return sCtx.error("池化操作输入张量至少需要二维，代表高度和宽度");
        }
        int nDims = spDim1.size();
        const int* pDimSizes = spDim1.data();

        int nBatchs = 1;
        int nInputHeight = pDimSizes[0];
        int nInputWidth = pDimSizes[1];
        if( nInputHeight < m_nPoolHeight || nInputWidth < m_nPoolWidth ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        int nLayers = 1;
        for( int i=2; i<nDims; i++ ) {
            nLayers *= pDimSizes[i];
        }

        int nInputCells = nInputWidth * nInputHeight * nLayers;

        m_nInputHeight = pDimSizes[0];
        m_nInputWidth = pDimSizes[1];
        m_nInputLayer = 1;
        int pOutDimSizes[nDims];
        for( int i=2; i<nDims; i++) {
            pOutDimSizes[i] = pDimSizes[i];
            m_nInputLayer *= pDimSizes[i];
        }
        m_nOutHeight = (m_nInputHeight - m_nPoolHeight) / m_nStrideHeight + 1;
        m_nOutWidth = (m_nInputWidth - m_nPoolWidth) / m_nStrideWidth + 1;
        m_nOutTensorSize = m_nOutHeight * m_nOutWidth * m_nInputLayer;
        m_nInputTensorSize = m_nInputHeight * m_nInputWidth * m_nInputLayer;
        pOutDimSizes[0] = m_nOutHeight;
        pOutDimSizes[1] = m_nOutWidth;
        return createVariable(SDimension(nDims, pOutDimSizes),spVarOut);
    }

private:
    int m_nPoolWidth;
    int m_nPoolHeight;
    int m_nStrideWidth;
    int m_nStrideHeight;

    int m_nBatchInSize;
    int m_nInputWidth;
    int m_nInputHeight;
    int m_nInputLayer;
    int m_nInputTensorSize;

    int m_nOutWidth;
    int m_nOutHeight;
    int m_nOutTensorSize;
};

static SNnOperatorRegister s_Register("pool", CNnOperator::createOperator<CPoolOperator>);

#endif//__SimpleWork_NN_Operators_CPoolOperator_h__