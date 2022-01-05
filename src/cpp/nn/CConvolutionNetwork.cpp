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

int CConvolutionNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    if( initWeights(spInTensor) != sCtx.Success() ) {
        return sCtx.Error();
    }

    if(spInTensor->getDataSize() != m_nInputData) {
        return sCtx.Error();
    }

    int nInputWidth = m_nInputWidth;
    int nInputLayers = m_nInputLayers;
    int nInputTensorSize = m_nInputTensorSize;

    int nConvs = m_nConvs;
    int nConvWidth = m_nConvWidth;
    int nConvHeight = m_nConvHeight;
    int nConvSize = nConvWidth * nConvHeight * nInputLayers; 

    int nOutputWidth = m_nOutWidth;
    int nOutputHeight = m_nOutHeight;
    int nOutputTensorSize = m_nOutTensorSize;

    int nInputHeightStep = nInputWidth * nInputLayers;
    int nInputWidthStep = nInputLayers;
    int nConvHeightStep = nConvWidth * nInputLayers;
    int nConvWidthStep = nInputLayers;

    double* pWeightArray = m_spWeights;
    double* pBaisArray = m_spBais;

    int nTensor = m_nTensor;
    if( STensor::createTensor<double>(spOutTensor, m_spOutDimVector, nTensor * m_nOutTensorSize) != sCtx.Success() ){
        return sCtx.Error("创建输出张量失败");
    }  
    double* pInArray = spInTensor->getDataPtr<double>();
    double* pOutArray = spOutTensor->getDataPtr<double>();
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
    m_spInTensor = spInTensor;
    m_spOutTensor = spOutTensor;
    return sCtx.Success();
}

int CConvolutionNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.Error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.Success() ) {
        return sCtx.Error(errCode, "创建输入偏差张量失败");
    }

    //
    // 学习率先固定
    //
    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nInputLayers = m_nInputLayers;
    int nInputTensorSize = m_nInputTensorSize;

    int nConvWidth = m_nConvWidth;
    int nConvHeight = m_nConvHeight;
    int nConvs = m_nConvs;
    int nConvSize = nConvWidth * nConvHeight * nInputLayers; 

    int nOutputWidth = m_nOutWidth;
    int nOutputHeight = m_nOutHeight;
    int nOutputTensorSize = m_nOutTensorSize;

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

    int nTensor = m_nTensor;
    double dLearnRate = 5.0 / nConvSize;
    double* pOutArray = spOutTensor->getDataPtr<double>();
    double* pOutputDeviationArray = spOutDeviation->getDataPtr<double>();
    double* pInArray = spInTensor->getDataPtr<double>();
    double* pInputDeviationArray = spInDeviation->getDataPtr<double>();
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
                    //      Z = 权重*X-偏置
                    //      Y = activation(Z)
                    //      delta = 目标 - Y
                    //      E = delta*delta/2 目标函数
                    //      derivationZ = d(E) / d(Z) = d(E)/d(delta) * d(delta)/d(Y) * d(F)/d(Z)
                    //      其中：
                    //          d(E)/d(delta) = pOutputDeviationArray[iOutput]
                    //          d(delta)/d(Y) = 1
                    //          d(Y)/d(Z) = deactivate(Y)
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
    if( (t++ / 2) % 10 == 0) {
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

int CConvolutionNetwork::initWeights(const STensor& spInTensor) {
    if( !m_spOutDimVector ) {
        STensor& spInDimVector = spInTensor->getDimVector();

        int nInDims = spInDimVector->getDataSize();
        int* pInDimSizes = spInDimVector->getDataPtr<int>();

        //
        // 维度小于3的张量，无法进行卷积运算
        //
        if(nInDims < 3) {
            return sCtx.Error("卷积网络的输入张量维度，必须大于等于3，其中第一个维度为张量个数，第二三个维度为卷积运算高和宽");
        }

        int nTensor = pInDimSizes[0];
        int nInputHeight = pInDimSizes[1];
        int nInputWidth = pInDimSizes[2];
        if( nInputHeight < m_nConvWidth || nInputWidth < m_nConvWidth ) {
            return sCtx.Error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        int nInputLayers = 1;
        for( int i=3; i<nInDims; i++ ) {
            nInputLayers *= pInDimSizes[i];
        }
        m_nInputWidth = nInputWidth;
        m_nInputHeight = nInputHeight;
        m_nInputTensorSize = m_nInputHeight*m_nInputWidth*nInputLayers;
        m_nInputData = nTensor*m_nInputTensorSize;

        m_nOutWidth = nInputWidth - m_nConvWidth + 1;
        m_nOutHeight = nInputHeight - m_nConvHeight + 1;
        m_nOutTensorSize = m_nOutWidth*m_nOutHeight*m_nConvs;

        m_nTensor = nTensor;
        m_nInputLayers = nInputLayers;

        int pOutDimSizes[4] = {nTensor, m_nOutHeight, m_nOutWidth, m_nConvs};
        if( STensor::createVector<int>(m_spOutDimVector, 4, pOutDimSizes) != sCtx.Success() ) {
            return sCtx.Error("创建输出张量的维度向量失败");
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
    }
    return sCtx.Success();
}
