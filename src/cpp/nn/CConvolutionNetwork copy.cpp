#include "CConvolutionNetwork.h"
#include "math.h"
#include "iostream"

SCtx CConvolutionNetwork::sCtx("CConvolutionNetwork");
int CConvolutionNetwork::createNetwork(int nWidth, int nHeight, int nConvs, SNeuralNetwork& spNetwork) {
    CPointer<CConvolutionNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_nConvWidth = nWidth;
    spConvolution->m_nConvHeight = nHeight;
    spConvolution->m_nConvs = nConvs;
    spConvolution->m_pActivator = CActivator::getReLU();
    spNetwork.setPtr(spConvolution.getPtr());
    return sCtx.Success();
}

int CConvolutionNetwork::eval(const PTensor& inputTensor, IVisitor<const PTensor&>* pOutputReceiver) {
    if(m_nInputWidth == 0) {
        if( initWeights(inputTensor) != sCtx.Success() ) {
            return sCtx.Error();
        }
    }

    if(inputTensor.nData != m_nInputData) {
        return sCtx.Error();
    }

    int nInputWidth = m_nInputWidth;
    int nInputLayers = m_nInputLayers;
    int nInputTensorSize = inputTensor.nData/inputTensor.pDimSizes[0];

    int nConvs = m_nConvs;
    int nConvWidth = m_nConvWidth;
    int nConvHeight = m_nConvHeight;
    int nConvSize = nConvWidth * nConvHeight * nInputLayers; 

    int nOutputWidth = nInputWidth - nConvWidth + 1;
    int nOutputHeight = m_nInputHeight - nConvHeight + 1;
    int nOutputTensorSize = nOutputWidth * nOutputHeight * nConvs;

    int nInputHeightStep = nInputWidth * nInputLayers;
    int nInputWidthStep = nInputLayers;
    int nConvHeightStep = nConvWidth * nInputLayers;
    int nConvWidthStep = nInputLayers;

    double* pWeightArray = m_spWeights;
    double* pBaisArray = m_spBais;

    int nTensor = inputTensor.pDimSizes[0];
    CTaker<double*> spOutputTensorArray(new double[nOutputTensorSize * nTensor], [](double* ptr) {
        delete[] ptr;
    });
    double* pInArray = inputTensor.pDoubleArray;
    double* pOutArray = spOutputTensorArray;
    int nOutHsize = nOutputWidth*nConvs;
    int nOutWsize = nConvs;
    for(int iTensor=0; iTensor<nTensor; iTensor++) {
        for( int iOutY = 0, iOutHAt=0; iOutY < nOutputHeight; iOutY++, iOutHAt+=nOutHsize ) {
            for( int iOutX = 0, iOutWAt=iOutHAt; iOutX < nOutputWidth; iOutX++, iOutWAt+=nOutWsize ) {
                double* pConvWeights = pWeightArray;
                for( int iConv = 0; iConv < nConvs; iConv++) {
                    double dConv = 0;
                    int iInputH = iOutY * nInputHeightStep + iOutX * nInputWidthStep;
                    for( int iConvY=0, iWeightH=0; iConvY<nConvHeight; iConvY++, iWeightH+=nConvHeightStep, iInputH+=nInputHeightStep) {
                        for( int iConvX=0, iWeightW=iWeightH, iInputW=iInputH; iConvX<nConvWidth; iConvX++, iWeightW+=nConvWidthStep, iInputW+=nInputWidthStep ) {
                            for( int iInputLayer=0, iWeightAt=iWeightW, iInputAt=iInputW; iInputLayer<nInputLayers; iInputLayer++, iWeightAt++, iInputAt++) {
                                dConv += DVV(pConvWeights, iWeightAt, nConvSize) * 
                                        DVV(pInArray, iInputAt, nInputTensorSize);
                            }
                        }
                    }
                    DVV(pOutArray, iOutWAt+iConv, nOutputTensorSize) = dConv - DVV(pBaisArray, iConv, nConvs);
                    pConvWeights += nConvSize;
                }
            }
        }
        m_pActivator->activate(nOutputTensorSize, pOutArray, pOutArray);
        pInArray += nInputTensorSize;
        pOutArray += nOutputTensorSize;
    }

    int dimSize[4] = { nTensor, nOutputHeight, nOutputWidth, nConvs };
    PTensor outTensor;
    outTensor.idType = inputTensor.idType;
    outTensor.nData = nOutputTensorSize * nTensor;
    outTensor.pDoubleArray = spOutputTensorArray;
    outTensor.nDims = 4;
    outTensor.pDimSizes = dimSize;
    return pOutputReceiver->visit(outTensor);
}

int CConvolutionNetwork::learn(const PTensor& inputTensor, SNeuralNetwork::ILearnCtx* pLearnCtx, PTensor* pInputDeviation) {

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

        CConvolutionNetwork* pNetwork;
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

int CConvolutionNetwork::initWeights(const PTensor& inputTensor) {
    //
    // 维度小于3的张量，无法进行卷积运算
    //
    if(inputTensor.nDims < 3) {
        return sCtx.Error("卷积网络的输入张量维度，必须大于等于3，其中第一个维度为张量个数，第二三个维度为卷积运算高和宽");
    }

    int nTensor = inputTensor.pDimSizes[0];
    int nInputHeight = inputTensor.pDimSizes[1];
    int nInputWidth = inputTensor.pDimSizes[2];
    int nInputLayers = inputTensor.nData / nInputHeight / nInputWidth / nTensor;
    if( nInputHeight < m_nConvWidth || nInputWidth < m_nConvWidth ) {
        return sCtx.Error("输入张量尺寸需要大于等于卷积核尺寸");
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
    // 基础权重最大值为 0.5 / 卷积核权重值的数量
    //
    int nConvSize = m_nConvWidth*m_nConvHeight*nInputLayers;
    double xWeight = 1.0/nConvSize;
    for(int i=0; i<nWeight; i++) {
         *(pWeights+i) = (rand() % 10000 / 10000.0) * xWeight;
    }
    for(int i=0; i<m_nConvs; i++ ){
        *pBais = 0;
    }

    m_nInputWidth = nInputWidth;
    m_nInputHeight = nInputHeight;
    m_nInputLayers = nInputLayers;
    m_nInputData = inputTensor.nData;
    return sCtx.Success();
}

int CConvolutionNetwork::learn(const PTensor& inputTensor, const PTensor& outputTensor, const PTensor& outputDeviation, PTensor* pInputDeviation) {
    CTaker<double*> spInputDeviationArray;
    double* pAllInputDerivationArray = nullptr;
    if(pInputDeviation) {
        pAllInputDerivationArray = pInputDeviation->pDoubleArray;
        memset(pAllInputDerivationArray,0,sizeof(double)*inputTensor.nData);
    }else{
        spInputDeviationArray.take(new double[inputTensor.nData], [](double* ptr) {
            delete[] ptr;
        });
        pAllInputDerivationArray = spInputDeviationArray;
    }

    //
    // 学习率先固定
    //
    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nInputLayers = m_nInputLayers;
    int nInputTensorSize = inputTensor.nData/inputTensor.pDimSizes[0];

    int nConvWidth = m_nConvWidth;
    int nConvHeight = m_nConvHeight;
    int nConvs = m_nConvs;
    int nConvSize = nConvWidth * nConvHeight * nInputLayers; 

    int nOutputWidth = nInputWidth - nConvWidth + 1;
    int nOutputHeight = nInputHeight - nConvHeight + 1;
    int nOutputTensorSize = outputTensor.nData/outputTensor.pDimSizes[0];

    int nInputHeightStep = nInputWidth * nInputLayers;
    int nInputWidthStep = nInputLayers;
    int nConvHeightStep = nConvWidth * nInputLayers;
    int nConvWidthStep = nInputLayers;

    double* pWeightArray = m_spWeights;
    double* pBaisArray = m_spBais;

    int nWeights = nConvSize*nConvs;
    CTaker<double*> spWeightDeviationArray(new double[nWeights], [](double* ptr) {
        delete[] ptr;
    });
    double* pWeightDerivationArray = spWeightDeviationArray;
    memset(pWeightDerivationArray, 0 ,sizeof(double)*nWeights);

    int nTensor = inputTensor.pDimSizes[0];
    double dLearnRate = 5.0 / nConvSize;
    double* pOutArray = outputTensor.pDoubleArray;
    double* pOutputDeviationArray = outputDeviation.pDoubleArray;
    double* pInArray = inputTensor.pDoubleArray;
    double* pInputDeviationArray = pAllInputDerivationArray;
    int nOutHsize = nOutputWidth*nConvs;
    int nOutWsize = nConvs;
    for(int iTensor=0; iTensor<nTensor; iTensor++) {

        double pDerivationZ[nOutputTensorSize];
        m_pActivator->deactivate(nOutputTensorSize, pOutArray, pOutputDeviationArray, pDerivationZ);
        for( int iOutY = 0, iOutHAt=0; iOutY < nOutputHeight; iOutY++, iOutHAt+=nOutHsize ) {
            for( int iOutX = 0, iOutWAt=iOutHAt; iOutX < nOutputWidth; iOutX++, iOutWAt+=nOutWsize ) {
                double* pConvWeights = pWeightArray;
                double* pConvWeightDeviations = pWeightDerivationArray;
                for( int iConv = 0; iConv < nConvs; iConv++) {

                    //
                    //  计算目标函数对当前输出值的偏导数
                    //      X = 输入
                    //      Y = 权重*X-偏置
                    //      F = activation(Y)
                    //      delta = 目标 - F
                    //      E = delta*delta/2 目标函数
                    //      derivationZ = d(E) / d(Y) = d(E)/d(delta) * d(delta)/d(F) * d(F)/d(y)
                    //      其中：
                    //          d(E)/d(delta) = pOutputDeviationArray[iOutput]
                    //          d(delta)/d(F) = 1
                    //          d(F)/d(y) = deactivate(y)
                    //
                    double derivationZ = DVV(pDerivationZ,iOutWAt+iConv,nOutputTensorSize);

                    //
                    //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
                    //  
                    double* pConvWeights = pWeightArray + iConv * nConvSize;
                    double* pConvWeightDeviations = pWeightDerivationArray + iConv * nConvSize;

                    int iInputH = iOutY * nInputHeightStep + iOutX * nInputWidthStep;
                    for( int iConvY=0, iWeightH=0; iConvY<nConvHeight; iConvY++, iWeightH+=nConvHeightStep, iInputH+=nInputHeightStep) {
                        for( int iConvX=0, iWeightW=iWeightH, iInputW=iInputH; iConvX<nConvWidth; iConvX++, iWeightW+=nConvWidthStep, iInputW+=nInputWidthStep ) {
                            for( int iInputLayer=0, iWeightAt=iWeightW, iInputAt=iInputW; iInputLayer<nInputLayers; iInputLayer++, iWeightAt++, iInputAt++) {
                                DVV(pInputDeviationArray,iInputAt,nInputTensorSize) += derivationZ * DVV(pConvWeights,iWeightAt,nConvSize);
                                DVV(pConvWeightDeviations,iWeightAt,nConvSize) += derivationZ * DVV(pInArray,iInputAt,nInputTensorSize);
                            }
                        }
                    }

                    //
                    //  偏置的偏导数刚好是输出的偏导数的负数，所以，下降梯度值为(-derivationZ)
                    //
                    DVV(pBaisArray,iConv,nConvs) -= (-derivationZ) * dLearnRate;
                    pConvWeightDeviations += nConvSize;
                    pConvWeights += nConvSize;
                }
            }
        }

        pOutArray += nOutputTensorSize;
        pOutputDeviationArray += nOutputTensorSize;
        pInArray += nInputTensorSize;
        pInputDeviationArray += nInputTensorSize;
    }

    double avgWeight = 0;
    double maxW = -100000;
    double minW = 100000;
    double avgDerivation = 0;
    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        DVV(pWeightArray, iWeight, nWeights) -= DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate;
        avgWeight += DVV(pWeightArray,iWeight,nWeights) / nWeights;
        avgDerivation += abs(DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate) / nWeights;
        if(maxW < DVV(pWeightArray,iWeight,nWeights)) {
            maxW = DVV(pWeightArray,iWeight,nWeights);
        }
        if(minW > DVV(pWeightArray,iWeight,nWeights)) {
            minW = DVV(pWeightArray,iWeight,nWeights);
        }
    }
    static int t = 0;
    if( (t++ / 2) % 20 == 0) {
        std::cout << "Conv " << nWeights << " ,Weight: " << minW << " ," << avgWeight <<" ," << maxW <<" , AD: " << avgDerivation << "\n";
    }
    /*
    //
    //  检查梯度下降后的值是否降低
    //
    struct CEvalCheck : IVisitor<const PTensor&> {
        int visit(const PTensor& t) {
            double delta0 = 0;
            for(int i=0; i<pOutputDeviation->nData; i++) {
                delta0 += pOutputDeviation->pDoubleArray[i]*pOutputDeviation->pDoubleArray[i];
            }

            double delta = 0;
            for(int i=0; i<pOutputDeviation->nData; i++) {
                delta += (t.pDoubleArray[i] - pOutput->pDoubleArray[i] + pOutputDeviation->pDoubleArray[i]) * (t.pDoubleArray[i] - pOutput->pDoubleArray[i] + pOutputDeviation->pDoubleArray[i]);
            }

            if( delta > delta0) {
                std::cout << ", Conv" << pOutput->nData << ": " << delta0 << " --> " << delta << ", ";
                //sCtx.Error("怎么下降，偏差越大？");
            }
            return sCtx.Success();
        }
        const PTensor* pOutput;
        const PTensor* pOutputDeviation;
    }evalCheck;
    evalCheck.pOutput = &outputTensor;
    evalCheck.pOutputDeviation = &outputDeviation;
    return eval(inputTensor, &evalCheck);
    */

    return sCtx.Success();
}
