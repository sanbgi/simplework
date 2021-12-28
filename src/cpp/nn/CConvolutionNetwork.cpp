#include "CConvolutionNetwork.h"

int CConvolutionNetwork::getCellNumber() {
    return m_nConvWidth*m_nConvHeight*m_nConvs;
}

int CConvolutionNetwork::createNetwork(int nWidth, int nHeight, int nConvs, SNeuralNetwork& spNetwork) {
    CPointer<CConvolutionNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_nConvWidth = nWidth;
    spConvolution->m_nConvHeight = nHeight;
    spConvolution->m_nConvs = nConvs;
    spNetwork.setPtr(spConvolution.getPtr());
    return SError::ERRORTYPE_SUCCESS;
}

void CConvolutionNetwork::activate(int nSize, double arrY[], double pOutputArray[]) {
    for(int i=0; i<nSize; i++) {
        pOutputArray[i] = arrY[i];
    }
}

void CConvolutionNetwork::deactivate(int nSize, double pOutputArray[], double arrDelta[], double arrDeirvatioY[]) {
    for(int i=0; i<nSize; i++) {
        arrDeirvatioY[i] = -arrDelta[i];
    }
}

int CConvolutionNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {
    if(m_nInputWidth == 0) {
        if( initWeights(inputTensor) != SError::ERRORTYPE_SUCCESS ) {
            return SError::ERRORTYPE_FAILURE;
        }
    }

    if(inputTensor.nData != m_nInputData) {
        return SError::ERRORTYPE_FAILURE;
    }

    int nConvs = m_nConvs;
    int nConvSize = m_nConvWidth * m_nConvHeight * m_nInputLayers; 
    int nConvWidth = m_nConvWidth;
    int nInputWidth = m_nInputWidth;
    int nInputLayers = m_nInputLayers;
    int nOutputWidth = m_nConvWidth - m_nConvWidth + 1;
    int nOutputHeight = m_nInputHeight - m_nConvHeight + 1;
    int nOutput = nOutputWidth*nOutputHeight*nConvs;
    double pOutputArray[nOutput];
    double* pInputArray = inputTensor.pDoubleArray;
    double* pWeights = m_spWeights;
    double* pBais = m_spBais;
    for( int iOutput=0; iOutput<nOutput; iOutput++) {
        int iConv = iOutput % nConvs;
        int iOutputX = iOutput / nConvs;
        int iOutputY = iOutputX / nOutputWidth;
        iOutputX = iOutputX % nOutputWidth;

        double* pConvWeights = pWeights + iConv * nConvSize;
        double dConv = 0;
        for( int iInputLayer=0; iInputLayer<nInputLayers; iInputLayer++) {
            for( int iConvY=0; iConvY<m_nConvHeight; iConvY++) {
                for( int iConvX=0; iConvX<m_nConvWidth; iConvX++ ) {
                    int iInputY = iOutputY+iConvY;
                    int iInputX = iOutputX+iConvX;
                    dConv += pConvWeights[(iConvY*nConvWidth+iConvX)*nInputLayers+iInputLayer] * 
                                pInputArray[(iInputY*nInputWidth+iInputX)*nInputLayers+iInputLayer];
                } 
            }
        }
        pOutputArray[iOutput] = dConv - pBais[iConv];
    }
    activate(nOutput, pOutputArray, pOutputArray);

    int dimSize[3] = { nOutputHeight, nOutputWidth, nConvs };
    PTensor outTensor;
    outTensor.idType = inputTensor.idType;
    outTensor.nData = nOutput;
    outTensor.pData = pOutputArray;
    outTensor.nDims = 3;
    outTensor.pDimSizes = dimSize;
    return pOutputReceiver->visit(outTensor);
}

int CConvolutionNetwork::learn(const PTensor& inputTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx) {

    struct COutputReceiver : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {

            struct CDeltaReceiver : IVisitor<const PTensor&> {
                int visit(const PTensor& t) {
                    return pNetwork->learn(*pInputTensor, *pOutputTensor, t, dInputWeight, pLearnCtx);
                }

                CConvolutionNetwork* pNetwork;
                double dInputWeight;
                SNeuralNetwork::ILearnCtx* pLearnCtx;
                const PTensor* pOutputTensor;
                const PTensor* pInputTensor;
            }deltaReceiver;
            deltaReceiver.pNetwork = this->pNetwork;
            deltaReceiver.dInputWeight = this->dInputWeight;
            deltaReceiver.pLearnCtx = this->pLearnCtx;
            deltaReceiver.pOutputTensor = &t;
            deltaReceiver.pInputTensor = this->pInputTensor;
            return pLearnCtx->getOutputTarget(t, &deltaReceiver);
        }

        CConvolutionNetwork* pNetwork;
        double dInputWeight;
        SNeuralNetwork::ILearnCtx* pLearnCtx;
        const PTensor* pInputTensor;
    }outputReceiver;
    outputReceiver.pNetwork = this;
    outputReceiver.dInputWeight = dInputWeight;
    outputReceiver.pLearnCtx = pLearnCtx;
    outputReceiver.pInputTensor = &inputTensor;
    return eval(inputTensor, &outputReceiver);
}

int CConvolutionNetwork::initWeights(const PTensor& inputTensor) {
    //
    // 维度小于2的张量，无法进行卷积运算
    //
    if(inputTensor.nDims < 2) {
        return SError::ERRORTYPE_FAILURE;
    }

    int nInputHeight = inputTensor.pDimSizes[0];
    int nInputWidth = inputTensor.pDimSizes[1];
    int nInputLayers = inputTensor.nData / nInputHeight / nInputWidth;
    if( nInputHeight < m_nConvWidth || nInputWidth < m_nConvWidth ) {
        return SError::ERRORTYPE_FAILURE;
    }

    int nWeight = m_nConvWidth*m_nConvHeight*nInputLayers*m_nConvs;
    double* pWeights = new double[nWeight];
    double* pBais = new double[m_nConvs];
    m_spWeights.take(pWeights, [](double* pWeights){
        delete[] pWeights;
    });
    m_spBais.take(pBais, [](double* pBais){
        delete[] pBais;
    });

    //
    // 初始化权重值，从[0-1]均匀分布（注意不是随机值）
    //
    for(int i=0; i<nWeight; i++) {
        *pWeights = (i+1)/(double)nWeight;
    }
    for(int i=0; i<m_nConvs; i++ ){
        *pBais = 0;
    }

    m_nInputWidth = nInputWidth;
    m_nInputHeight = nInputHeight;
    m_nInputLayers = nInputLayers;
    m_nInputData = inputTensor.nData;
    return SError::ERRORTYPE_SUCCESS;
}

int CConvolutionNetwork::learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& deltaTensor, double dInputWeight, SNeuralNetwork::ILearnCtx* pLearnCtx) {

    double expectInputDelta[inputTensor.nData];
    memset(expectInputDelta,0,sizeof(double)*inputTensor.nData);

    //
    // 将执行体放在括号中，达到节约栈内存目的(因为括号中的内存可以在回调之前释放)
    //
    {
        //
        // 学习率先固定
        //
        double dLearnRate = 0.1;

        //
        //  计算目标函数相对于Y值的偏导数
        //
        double derivationYs[outputTensor.nData];
        deactivate(outputTensor.nData, outputTensor.pDoubleArray, deltaTensor.pDoubleArray, derivationYs);

        int nConvs = m_nConvs;
        int nConvSize = m_nConvWidth * m_nConvHeight * m_nInputLayers; 
        int nConvWidth = m_nConvWidth;
        int nConvHeight = m_nConvHeight;

        int nInputWidth = m_nInputWidth;
        int nInputLayers = m_nInputLayers;

        int nOutputWidth = m_nInputWidth - m_nConvWidth + 1;
        int nOutputHeight = m_nInputHeight - m_nConvHeight + 1;
        int nOutput = nOutputWidth*nOutputHeight*nConvs;

        double* pInputArray = inputTensor.pDoubleArray;
        double* pWeights = m_spWeights;
        double* pBais = m_spBais;
        for( int iOutput=0; iOutput<nOutput; iOutput++) {
            int iConv = iOutput % nConvs;
            int iOutputX = iOutput / nConvs;
            int iOutputY = iOutputX / nOutputWidth;
            iOutputX = iOutputX % nOutputWidth;

            double* pConvWeights = pWeights + iConv * nConvSize;
            double derivationY = derivationYs[iOutput];
            for( int iInputLayer=0; iInputLayer<nInputLayers; iInputLayer++) {
                for( int iConvY=0; iConvY<nConvHeight; iConvY++) {
                    for( int iConvX=0; iConvX<nConvWidth; iConvX++ ) {
                        int iInputY = iOutputY+iConvY;
                        int iInputX = iOutputX+iConvX;
                        int iWeight = (iConvY*nConvWidth+iConvX)*nInputLayers+iInputLayer;
                        int iInput = (iInputY*nInputWidth+iInputX)*nInputLayers+iInputLayer;
                        expectInputDelta[iInput] -= derivationY * pConvWeights[iWeight];
                        pConvWeights[iWeight] -= derivationY*pInputArray[iInput]*dLearnRate;
                    }
                }
            }
            pBais[iOutput] -= (-derivationY) * dLearnRate;
        }
    }

    PTensor expectInputDeltaTensor = inputTensor;
    expectInputDeltaTensor.pDoubleArray = expectInputDelta;
    return pLearnCtx->pubInputDelta(expectInputDeltaTensor);
}