#include "CPoolNetwork.h"

SCtx CPoolNetwork::sCtx("CPoolNetwork");
int CPoolNetwork::createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) {
    CPointer<CPoolNetwork> spPool;
    CObject::createObject(spPool);
    spPool->m_nWidth = nWidth;
    spPool->m_nHeight = nHeight;
    spPool->m_nStrideWidth = nStrideWidth;
    spPool->m_nStrideHeight = nStrideHeight;
    spNetwork.setPtr(spPool.getPtr());
    return sCtx.success();
}

int CPoolNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {

    if(initNetwork(spInTensor) != sCtx.success()) {
        return sCtx.error();
    }

    if(spInTensor->getDataSize() != m_nTensor * m_nInputTensorSize) {
        return sCtx.error();
    }

    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = m_nOutWidth;
    int nOutHeight = m_nOutHeight;
    int nTensor = m_nTensor;
    int nLayer = m_nInputLayer;
    int nInputTensorSize = m_nInputTensorSize;
    int nOutputTensorSize = m_nOutTensorSize;

    int nInputHstep = m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * m_nStrideHeight;
    int nInStrideWstep = nInputWstep * m_nStrideWidth;

    int nOutHstep = m_nOutWidth * nLayer;
    int nOutWstep = nLayer;

    struct CItOutVariables {
        double* pIn;
        double* pOut;
    }it = {
        spInTensor->getDataPtr<double>(),
        m_spOutTensor->getDataPtr<double>(),
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

                    double dMax = (*it.pIn);

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
    
    m_spInTensor = spInTensor;
    spOutTensor = m_spOutTensor;
    return sCtx.success();
}

int CPoolNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    double* pDeviationInputArray = spInDeviation->getDataPtr<double>();
    memset(pDeviationInputArray,0,sizeof(double)*spInTensor->getDataSize());

    int nLayer = m_nInputLayer;
    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nInputLayer = m_nInputLayer;
    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = m_nOutWidth;
    int nOutHeight = m_nOutHeight;

    int nTensor = m_nTensor;
    int nInputTensorSize = m_nInputTensorSize;
    int nOutputTensorSize = m_nOutTensorSize;

    int nInputHstep = m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * m_nStrideHeight;
    int nInStrideWstep = nInputWstep * m_nStrideWidth;
    
    int nOutHstep = nOutWidth * nLayer;
    int nOutWstep = nLayer;

    struct CItOutVariables {
        double* pIn;
        double* pInDeviation;
        double* pOutDeviation;
    }it = {
        spInTensor->getDataPtr<double>(),
        spInDeviation->getDataPtr<double>(),
        spOutDeviation->getDataPtr<double>(),
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

                    double dMax = (*it.pIn);
                    double* pExpectDelta = it.pInDeviation;

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
    
    return sCtx.success();
}

int CPoolNetwork::initNetwork(const STensor& inputTensor) {
    if( !m_spOutDimVector ) {

        STensor& spInDimVector = inputTensor->getDimVector();
        int nDims = spInDimVector->getDataSize();
        int* pDimSizes = spInDimVector->getDataPtr<int>();
        if(nDims < 3) {
            return sCtx.error("池化层输入张量维度需要大于等于3，其中第一个维度为实际张量个数");
        }
        m_nTensor = pDimSizes[0];
        m_nInputHeight = pDimSizes[1];
        m_nInputWidth = pDimSizes[2];
        m_nInputLayer = 1;


        int pOutDimSizes[nDims];
        for( int i=3; i<nDims; i++) {
            pOutDimSizes[i] = pDimSizes[i];
            m_nInputLayer *= pDimSizes[i];
        }
        m_nOutHeight = (m_nInputHeight - m_nHeight) / m_nStrideHeight + 1;
        m_nOutWidth = (m_nInputWidth - m_nWidth) / m_nStrideWidth + 1;
        m_nOutTensorSize = m_nOutHeight * m_nOutWidth * m_nInputLayer;
        m_nInputTensorSize = m_nInputHeight * m_nInputWidth * m_nInputLayer;
        pOutDimSizes[0] = m_nTensor;
        pOutDimSizes[1] = m_nOutHeight;
        pOutDimSizes[2] = m_nOutWidth;
        if( STensor::createVector(m_spOutDimVector, nDims, pOutDimSizes) != sCtx.success() ) {
            return sCtx.error("创建输出张量的维度张量失败");
        }

        if( STensor::createTensor<double>(m_spOutTensor, m_spOutDimVector, m_nOutTensorSize * m_nTensor) != sCtx.success() ) {
            return sCtx.error("创建输出张量失败");
        }
    }
    return sCtx.success();
}