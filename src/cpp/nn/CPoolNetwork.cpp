#include "CPoolNetwork.h"

int CPoolNetwork::createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNeuralNetwork& spNetwork) {
    CPointer<CPoolNetwork> spPool;
    CObject::createObject(spPool);
    spPool->m_nWidth = nWidth;
    spPool->m_nHeight = nHeight;
    spPool->m_nStrideWidth = nStrideWidth;
    spPool->m_nStrideHeight = nStrideHeight;
    spNetwork.setPtr(spPool.getPtr());
    return SError::ERRORTYPE_SUCCESS;
}

int CPoolNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {

    if(initNetwork(inputTensor) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }

    if(m_isTransparent) {
        return pOutputReceiver->visit(inputTensor);
    }

    int nLayer = 1;
    int pDimSizes[inputTensor.nDims];
    for(int i=2; i<inputTensor.nDims; i++) {
        pDimSizes[i] = inputTensor.pDimSizes[i];
        nLayer *= pDimSizes[i];
    }

    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = (m_nInputWidth - m_nWidth + 1) / m_nStrideWidth;
    int nOutHeight = (m_nInputHeight - m_nHeight + 1) / m_nStrideHeight;
    double* pInputArray = inputTensor.pDoubleArray;
    double pOutputArray[nOutWidth*nOutHeight*nLayer];
    for( int iOutHeight = 0; iOutHeight <= nOutHeight; iOutHeight++ ) {
        for( int iOutWidth = 0; iOutWidth <= nOutWidth; iOutWidth++) {
            int iInputHeight = iOutHeight*m_nStrideHeight;
            int iInputWidth = iOutWidth*m_nStrideWidth;
            for(int iLayer=0; iLayer<nLayer; iLayer++) {
                double dMax = pInputArray[(iInputHeight*nInputWidth+iInputWidth)*nLayer+iLayer];
                for( int iPoolHeight=0; iPoolHeight<nPoolHeight; iPoolHeight++) {
                    for( int iPoolWidth=0; iPoolWidth<nPoolWidth; iPoolWidth++) {
                        int iHeight = iInputHeight+iPoolHeight;
                        int iWidth = iInputWidth+iPoolWidth;
                        double dValue = pInputArray[(iHeight*nInputWidth+iWidth)*nLayer+iLayer];
                        if( dValue > dMax) {
                            dMax = dValue;
                        }
                    }
                }
                pOutputArray[(iOutHeight*nOutWidth+iOutWidth)*nLayer+iLayer] = dMax;
            }
        }
    }

    PTensor tensorOutput;
    tensorOutput.idType = inputTensor.idType;
    tensorOutput.nData = nOutWidth*nOutHeight*nLayer;
    tensorOutput.pData = pOutputArray;
    tensorOutput.nDims = inputTensor.nDims;
    tensorOutput.pDimSizes = pDimSizes;
    return pOutputReceiver->visit(tensorOutput);
}

int CPoolNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx) {
    if(initNetwork(inputTensor) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }
    
    struct CDeltaReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {
            return pNetwork->learn(*pInputTensor, t, pLearnCtx);
        }
        SNeuralNetwork::ILearnCtx* pLearnCtx;
        CPoolNetwork* pNetwork;
        const PTensor* pInputTensor;
        double dInputWeight;
    }deltaReceiver;
    deltaReceiver.pLearnCtx = pLearnCtx;
    deltaReceiver.pNetwork = this;
    deltaReceiver.pInputTensor = &inputTensor;
    return pLearnCtx->getOutputDelta(inputTensor, &deltaReceiver); 
}

int CPoolNetwork::learn(const PTensor& inputTensor, const PTensor& deltaTensor, SNeuralNetwork::ILearnCtx* pLearnCtx) {
    if(m_isTransparent) {
        return pLearnCtx->setInputDelta(deltaTensor);
    }

    double pExpectInputDelta[inputTensor.nData];
    memset(pExpectInputDelta,0,sizeof(double)*inputTensor.nData);
    {
        int nInputWidth = m_nInputWidth;
        int nInputHeight = m_nInputHeight;
        int nInputLayer = m_nInputLayer;
        int nPoolWidth = m_nWidth;
        int nPoolHeight = m_nHeight;
        int nOutWidth = (m_nInputWidth - m_nWidth + 1) / m_nStrideWidth;
        int nOutHeight = (m_nInputHeight - m_nHeight + 1) / m_nStrideHeight;
        double* pInputArray = inputTensor.pDoubleArray;
        double* pDeltaArray = deltaTensor.pDoubleArray;
        for( int iOutHeight = 0; iOutHeight <= nOutHeight; iOutHeight++ ) {
            for( int iOutWidth = 0; iOutWidth <= nOutWidth; iOutWidth++) {
                int iInputHeight = iOutHeight*m_nStrideHeight;
                int iInputWidth = iOutWidth*m_nStrideWidth;
                for(int iLayer=0; iLayer<nInputLayer; iLayer++) {
                    double dMax = pInputArray[(iInputHeight*nInputWidth+iInputWidth)*nInputLayer+iLayer];
                    double* pExpectDelta = nullptr;
                    for( int iPoolHeight=0; iPoolHeight<nPoolHeight; iPoolHeight++) {
                        for( int iPoolWidth=0; iPoolWidth<nPoolWidth; iPoolWidth++) {
                            int iHeight = iInputHeight+iPoolHeight;
                            int iWidth = iInputWidth+iPoolWidth;
                            double dValue = pInputArray[(iHeight*nInputWidth+iWidth)*nInputLayer+iLayer];
                            if( dValue > dMax) {
                                dMax = dValue;
                                pExpectDelta = &pExpectInputDelta[(iHeight*nInputWidth+iWidth)*nInputLayer+iLayer];
                            }
                        }
                    }
                    if(pExpectDelta) {
                        *pExpectDelta = pDeltaArray[(iOutHeight*nOutWidth+iOutWidth)*nInputLayer+iLayer];
                    }
                }
            }
        }
    }
    PTensor expectInputDeltaTensor = inputTensor;
    expectInputDeltaTensor.pDoubleArray = pExpectInputDelta;
    return pLearnCtx->setInputDelta(expectInputDeltaTensor);
}

int CPoolNetwork::initNetwork(const PTensor& inputTensor) {
    if(m_nInputHeight == 0) {
        m_nInputHeight = inputTensor.pDimSizes[0];
        if(inputTensor.nDims < 2 || inputTensor.pDimSizes[0] < m_nHeight || inputTensor.pDimSizes[1] < m_nWidth ) {
            m_isTransparent = true;
        }else{
            m_nInputWidth = inputTensor.pDimSizes[1];
            m_nInputLayer = inputTensor.nData/m_nInputWidth/m_nInputHeight;
        }
    }
    return SError::ERRORTYPE_SUCCESS;
}