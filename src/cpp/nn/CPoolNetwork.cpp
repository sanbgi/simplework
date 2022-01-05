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
    return sCtx.Success();
}

int CPoolNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {

    if(initNetwork(spInTensor) != sCtx.Success()) {
        return sCtx.Error();
    }

    if( STensor::createTensor<double>(spOutTensor, m_spOutDimVector, m_nOutTensorSize * m_nTensor) != sCtx.Success() ) {
        return sCtx.Error("创建输出张量失败");
    }

    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = m_nOutWidth;
    int nOutHeight = m_nOutHeight;
    int nTensor = m_nTensor;
    int nLayer = m_nInputLayer;
    int nInputTensorSize = m_nInputTensorSize;
    int nOutputTensorSize = m_nOutTensorSize;
    {
        int nInputHstep = m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * m_nStrideHeight;
        int nInStrideWstep = nInputWstep * m_nStrideWidth;

        int nOutHstep = m_nOutWidth * nLayer;
        int nOutWstep = nLayer;
        
        double* pInArray = spInTensor->getDataPtr<double>();
        double* pOutArray = spOutTensor->getDataPtr<double>();
        for(int iTensor = 0; iTensor < nTensor; iTensor++ ) {
            for( int iOutH=0, iInHAt=0, iOutHAt=0; iOutH < nOutHeight; iOutH++, iInHAt+=nInStrideHstep, iOutHAt += nOutHstep ) {
                for( int iOutW = 0, iInWAt=iInHAt, iOutWAt=iOutHAt; iOutW < nOutWidth; iOutW++, iInWAt+=nInStrideWstep, iOutWAt+=nOutWstep) {
                    for(int iLayer=0, iPoolInLAt=iInWAt; iLayer<nLayer; iLayer++, iPoolInLAt++) {
                        double dMax = DVV(pInArray, iPoolInLAt, nInputTensorSize);
                        for( int iPoolH=0, iPoolInHAt=iPoolInLAt; iPoolH<nPoolHeight; iPoolH++, iPoolInHAt+=nInputHstep) {
                            for( int iPoolW=0, iPoolInWAt=iPoolInHAt; iPoolW<nPoolWidth; iPoolW++, iPoolInWAt+=nInputWstep) {
                                double dValue = DVV(pInArray, iPoolInWAt, nInputTensorSize);
                                if( dValue > dMax) {
                                    dMax = dValue;
                                }
                            }
                        }
                        DVV(pOutArray, iOutWAt+iLayer, nOutputTensorSize) = dMax;
                    }
                }
            }
            pInArray+=nInputTensorSize;
            pOutArray+=nOutputTensorSize;
        }
    }
    m_spInTensor = spInTensor;
    m_spOutTensor = spOutTensor;
    return sCtx.Success();
}

int CPoolNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.Error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.Success() ) {
        return sCtx.Error(errCode, "创建输入偏差张量失败");
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

    double* pInArray = spInTensor->getDataPtr<double>();
    double* pOutDeltaArray = spOutDeviation->getDataPtr<double>();
    double* pInDeltaArray = pDeviationInputArray;
    for(int iTensor = 0; iTensor < nTensor; iTensor++ ) {
        for( int iOutH=0, iInHAt=0, iOutHAt=0; iOutH < nOutHeight; iOutH++, iInHAt+=nInStrideHstep, iOutHAt += nOutHstep ) {
            for( int iOutW = 0, iInWAt=iInHAt, iOutWAt=iOutHAt; iOutW < nOutWidth; iOutW++, iInWAt+=nInStrideWstep, iOutWAt+=nOutWstep) {
                for(int iLayer=0, iPoolInLAt=iInWAt; iLayer<nLayer; iLayer++, iPoolInLAt++) {
                    double dMax = DVV(pInArray, iPoolInLAt, nInputTensorSize);
                    double* pExpectDelta = nullptr;
                    for( int iPoolH=0, iPoolInHAt=iPoolInLAt; iPoolH<nPoolHeight; iPoolH++, iPoolInHAt+=nInputHstep) {
                        for( int iPoolW=0, iPoolInWAt=iPoolInHAt; iPoolW<nPoolWidth; iPoolW++, iPoolInWAt+=nInputWstep) {
                            double dValue = DVV(pInArray, iPoolInWAt, nInputTensorSize);
                            if( dValue > dMax) {
                                dMax = dValue;
                                pExpectDelta = &DVV(pInDeltaArray, iPoolInWAt, nInputTensorSize);
                            }
                        }
                    }
                    if(pExpectDelta) {
                        *pExpectDelta = DVV(pOutDeltaArray,iOutWAt+iLayer, nOutputTensorSize);
                    }
                }
            }
        }
        pInArray+=nInputTensorSize;
        pInDeltaArray+=nInputTensorSize;
        pOutDeltaArray+=nOutputTensorSize;
    }
    return sCtx.Success();
}

int CPoolNetwork::initNetwork(const STensor& inputTensor) {
    if( !m_spOutDimVector ) {

        STensor& spInDimVector = inputTensor->getDimVector();
        int nDims = spInDimVector->getDataSize();
        int* pDimSizes = spInDimVector->getDataPtr<int>();
        if(nDims < 3) {
            return sCtx.Error("池化层输入张量维度需要大于等于3，其中第一个维度为实际张量个数");
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
        if( STensor::createVector(m_spOutDimVector, nDims, pOutDimSizes) != sCtx.Success() ) {
            return sCtx.Error("创建输出张量的维度张量失败");
        }
    }
    return sCtx.Success();
}