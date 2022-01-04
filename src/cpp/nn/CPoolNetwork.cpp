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

int CPoolNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {

    if(initNetwork(inputTensor) != sCtx.Success()) {
        return sCtx.Error();
    }

    if(m_isTransparent) {
        return pOutputReceiver->visit(inputTensor);
    }

    int nLayer = 1;
    int pDimSizes[inputTensor.nDims];
    for(int i=0; i<inputTensor.nDims; i++) {
        pDimSizes[i] = inputTensor.pDimSizes[i];
        if(i>2) {
            nLayer *= pDimSizes[i];
        }
    }

    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = (m_nInputWidth - m_nWidth) / m_nStrideWidth + 1;
    int nOutHeight = (m_nInputHeight - m_nHeight) / m_nStrideHeight + 1;
    pDimSizes[1] = nOutHeight;
    pDimSizes[2] = nOutWidth;
    int nTensor = inputTensor.pDimSizes[0];
    int nInputTensorSize = inputTensor.nData/inputTensor.pDimSizes[0];
    int nOutputTensorSize = nOutWidth*nOutHeight*nLayer;
    CTaker<double*> spOutputArray(new double[nTensor*nOutputTensorSize], [](double* ptr) {
        delete[] ptr;
    });
    {
        int nInputHstep = m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * m_nStrideHeight;
        int nInStrideWstep = nInputWstep * m_nStrideWidth;

        int nOutHstep = nOutWidth * nLayer;
        int nOutWstep = nLayer;
        
        double* pInArray = inputTensor.pDoubleArray;
        double* pOutArray = spOutputArray;
        for(int iTensor = 0; iTensor < nTensor; iTensor++, pInArray+=nInputTensorSize, pOutArray+=nOutputTensorSize ) {
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
        }
    }

    PTensor tensorOutput;
    tensorOutput.idType = inputTensor.idType;
    tensorOutput.nData = nTensor*nOutWidth*nOutHeight*nLayer;
    tensorOutput.pDoubleArray = spOutputArray;
    tensorOutput.nDims = inputTensor.nDims;
    tensorOutput.pDimSizes = pDimSizes;
    return pOutputReceiver->visit(tensorOutput);
}

int CPoolNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx, PTensor* pInputDeviation) {

    struct COutputReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& outputTensor) {

            int nOutputData = outputTensor.nData;
            CTaker<double*> spOutputDeviation(new double[nOutputData], [](double* ptr) {
                delete[] ptr;
            });
            PTensor outputDeviation = outputTensor;
            outputDeviation.pDoubleArray = spOutputDeviation;
            if( pLearnCtx->getOutputDeviation(outputTensor, outputDeviation) != sCtx.Success() ) {
                return sCtx.Error();
            }

            return pNetwork->learn(*pInputTensor, outputTensor, outputDeviation, pInputDeviation);
        }

        CPoolNetwork* pNetwork;
        SNeuralNetwork::ILearnCtx* pLearnCtx;
        const PTensor* pInputTensor;
        PTensor* pInputDeviation;
    }outputReceiver;
    outputReceiver.pNetwork = this;
    outputReceiver.pLearnCtx = pLearnCtx;
    outputReceiver.pInputTensor = &inputTensor;
    outputReceiver.pInputDeviation = pInputDeviation;
    return eval(inputTensor, &outputReceiver);
}

int CPoolNetwork::learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& outputDeviation, PTensor* pInputDeviation) {
    if(pInputDeviation == nullptr) {
        return sCtx.Success();
    }
    
    if(m_isTransparent) {
        if(pInputDeviation)
        mempcpy(pInputDeviation->pDoubleArray, outputDeviation.pDoubleArray, sizeof(double)*inputTensor.nData);
        return sCtx.Success();
    }

    double* pDeviationInputArray = pInputDeviation->pDoubleArray;
    memset(pDeviationInputArray,0,sizeof(double)*inputTensor.nData);

    int nLayer = m_nInputLayer;
    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nInputLayer = m_nInputLayer;
    int nPoolWidth = m_nWidth;
    int nPoolHeight = m_nHeight;
    int nOutWidth = (m_nInputWidth - m_nWidth) / m_nStrideWidth + 1;
    int nOutHeight = (m_nInputHeight - m_nHeight) / m_nStrideHeight + 1;

    int nTensor = inputTensor.pDimSizes[0];
    int nInputTensorSize = inputTensor.nData/inputTensor.pDimSizes[0];
    int nOutputTensorSize = outputTensor.nData/outputTensor.pDimSizes[0];
    int nDeltaTensorSize = outputDeviation.nData/outputDeviation.pDimSizes[0];

    {
        int nInputHstep = m_nInputWidth * nLayer;
        int nInputWstep = nLayer;

        int nInStrideHstep = nInputHstep * m_nStrideHeight;
        int nInStrideWstep = nInputWstep * m_nStrideWidth;
        
        int nOutHstep = nOutWidth * nLayer;
        int nOutWstep = nLayer;

        double* pInArray = inputTensor.pDoubleArray;
        double* pOutDeltaArray = outputDeviation.pDoubleArray;
        double* pInDeltaArray = pDeviationInputArray;
        for(int iTensor = 0; iTensor < nTensor; iTensor++, pInArray+=nInputTensorSize, pInDeltaArray+=nInputTensorSize, pOutDeltaArray+=nOutputTensorSize ) {
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
        }
    }

    return sCtx.Success();
}

int CPoolNetwork::initNetwork(const PTensor& inputTensor) {
    if(m_nInputHeight == 0) {
        if(inputTensor.nDims < 3) {
            return sCtx.Error("池化层输入张量维度需要大于等于3，其中第一个维度为实际张量个数");
        }
        int nTensor = inputTensor.pDimSizes[0];
        m_nInputHeight = inputTensor.pDimSizes[1];
        m_nInputWidth = inputTensor.pDimSizes[2];
        if(m_nInputHeight < m_nHeight || m_nInputWidth < m_nWidth ) {
            m_isTransparent = true;
        }else{
            m_nInputLayer = inputTensor.nData/m_nInputWidth/m_nInputHeight/nTensor;
        }
    }
    return sCtx.Success();
}