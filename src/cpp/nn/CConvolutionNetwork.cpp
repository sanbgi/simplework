#include "CConvolutionNetwork.h"
#include "math.h"
#include "iostream"

SCtx CConvolutionNetwork::sCtx("CConvolutionNetwork");
int CConvolutionNetwork::createNetwork(int nWidth, int nHeight, int nConvs, const char* szActivator, SNeuralNetwork& spNetwork) {
    CPointer<CConvolutionNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_nConvWidth = nWidth;
    spConvolution->m_nConvHeight = nHeight;
    spConvolution->m_nConvs = nConvs;
    spConvolution->m_pActivator = CActivator::getActivation(szActivator);
    if(spConvolution->m_pActivator == nullptr) {
        return sCtx.error((std::string("不支持的激活函数名: ") + szActivator).c_str());
    }
    spNetwork.setPtr(spConvolution.getPtr());
    return sCtx.success();
}

int CConvolutionNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    if( initWeights(spInTensor) != sCtx.success() ) {
        return sCtx.error();
    }

    if(spInTensor->getDataSize() != m_nInputData) {
        return sCtx.error();
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
    if( STensor::createTensor<double>(spOutTensor, m_spOutDimVector, nTensor * m_nOutTensorSize) != sCtx.success() ){
        return sCtx.error("创建输出张量失败");
    }
    int nOutHstep = nOutputWidth*nConvs;
    int nOutWstep = nConvs;
    struct CItOutVariables {
        double* pIn;
        double* pOut;
        double* pWeights;
        double* pBais;
    }it = {
        spInTensor->getDataPtr<double>(),
        spOutTensor->getDataPtr<double>(),
        m_spWeights,
        m_spBais
    };

    double dConv;
    int iTensor, iOutY, iOutX, iConv, iConvY, iConvX, iLayer;
    CItOutVariables varTBackup, varOYBackup, varOXBackup, varConvBackup, varConvYBackup, varConvXBackup;
    for(iTensor=0; iTensor<nTensor; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pOut,
        };
        for(iOutY=0; iOutY < nOutputHeight; iOutY++) {
            varOYBackup.pIn = it.pIn;
            varOYBackup.pOut = it.pOut;
            for(iOutX=0; iOutX < nOutputWidth; iOutX++) {
                varOXBackup.pIn = it.pIn;
                varOXBackup.pOut = it.pOut;
                varOXBackup.pWeights = it.pWeights;
                varOXBackup.pBais = it.pBais;
                for(iConv = 0; iConv < nConvs; iConv++) {
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pWeights = it.pWeights;

                    //初始化卷积结果
                    dConv = 0;
                    for( iConvY=0; iConvY<nConvHeight; iConvY++) {
                        varConvYBackup.pIn = it.pIn;
                        varConvYBackup.pWeights = it.pWeights;
                        for(iConvX=0; iConvX<nConvWidth; iConvX++) {
                            varConvXBackup.pIn = it.pIn;
                            varConvXBackup.pWeights = it.pWeights;
                            for( int iLayer=0; iLayer<nInputLayers; iLayer++) {
                                //卷积结果为乘积结果求和
                                dConv += (*it.pWeights) * (*it.pIn);
                                it.pIn++;
                                it.pWeights++;
                            }
                            it.pIn = varConvXBackup.pIn + nInputWidthStep;
                            it.pWeights = varConvXBackup.pWeights + nConvWidthStep;
                        }
                        it.pIn = varConvYBackup.pIn + nInputHeightStep;
                        it.pWeights = varConvYBackup.pWeights + nConvHeightStep;
                    }
                    //卷积结果减去偏置
                    (*it.pOut) = dConv - (*it.pBais);

                    it.pIn = varConvBackup.pIn;
                    it.pOut++;
                    it.pWeights = varConvBackup.pWeights + nConvSize;
                    it.pBais++;
                }
                it.pWeights = varOXBackup.pWeights;
                it.pBais = varOXBackup.pBais;
                it.pOut = varOXBackup.pOut + nOutWstep;
                it.pIn = varOXBackup.pIn + nInputWidthStep;
            }
            it.pIn = varOYBackup.pIn + nInputHeightStep;
            it.pOut = varOYBackup.pOut + nOutHstep;
        }
        //
        // 激活结果值
        //
        m_pActivator->activate(nOutputTensorSize, varTBackup.pOut, varTBackup.pOut);
        it.pIn = varTBackup.pIn + nInputTensorSize;
        it.pOut = varTBackup.pOut + nOutputTensorSize;
    }

    m_spInTensor = spInTensor;
    m_spOutTensor = spOutTensor;
    return sCtx.success();
}

int CConvolutionNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
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

    #ifdef _DEBUG
    double avgOutDerivation = 0;
    double avgWeight = 0;
    double maxW = -100000;
    double minW = 100000;
    double avgDerivation = 0;
    double avgBais = 0;
    #endif//_DEBUG

    int nTensor = m_nTensor;
    double dLearnRate = 5.0 / nConvSize;
    int nOutHstep = nOutputWidth*nConvs;
    int nOutWstep = nConvs;
    
    struct CItOutVariables {
        double* pIn;
        double* pInDeviation;
        double* pOut;
        double* pOutDeviation;
        double* pWeights;
        double* pWeightDerivation;
        double* pBais;
        double* pZDeviatioin;
    }it = {
        spInTensor->getDataPtr<double>(),
        spInDeviation->getDataPtr<double>(),
        spOutTensor->getDataPtr<double>(),
        spOutDeviation->getDataPtr<double>(),
        m_spWeights,
        pWeightDerivationArray,
        m_spBais
    };
    double derivationZ;
    double pDerivationZ[nOutputTensorSize];
    int iTensor, iOutY, iOutX, iConv, iConvY, iConvX, iLayer;
    CItOutVariables varTBackup, varOYBackup, varOXBackup, varConvBackup, varConvYBackup, varConvXBackup;
    for(iTensor=0; iTensor<nTensor; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pInDeviation,
            it.pOut,
            it.pOutDeviation
        };

        m_pActivator->deactivate(nOutputTensorSize, it.pOut, it.pOutDeviation, pDerivationZ);
        it.pZDeviatioin = pDerivationZ;
        for(iOutY=0; iOutY < nOutputHeight; iOutY++) {
            varOYBackup.pIn = it.pIn;
            varOYBackup.pInDeviation = it.pInDeviation;
            varOYBackup.pZDeviatioin = it.pZDeviatioin;
            for(iOutX=0; iOutX < nOutputWidth; iOutX++) {
                varOXBackup.pWeights = it.pWeights;
                varOXBackup.pWeightDerivation = it.pWeightDerivation;
                varOXBackup.pBais = it.pBais;
                varOXBackup.pZDeviatioin = it.pZDeviatioin;
                for(iConv = 0; iConv < nConvs; iConv++) {
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pInDeviation = it.pInDeviation;
                    varConvBackup.pWeights = it.pWeights;
                    varConvBackup.pWeightDerivation = it.pWeightDerivation;

                    #ifdef _DEBUG
                    avgOutDerivation += abs(it.pOutDeviation[(iOutY*nOutWstep+iOutX)+iConv]) / nOutputTensorSize / nTensor ;
                    #endif//_DEBUG

                    //
                    //  计算目标函数对当前输出值的偏导数
                    //      X = 输入
                    //      Z = 权重*X-偏置
                    //      Y = activation(Z)
                    //      delta = 目标 - Y
                    //      E = delta*delta/2 目标函数
                    //      derivationZ = d(E) / d(Z) = d(E)/d(delta) * d(delta)/d(Y) * d(F)/d(Z)
                    //      其中：
                    //          d(E)/d(delta) = pOutDeviation[iOutput]
                    //          d(delta)/d(Y) = 1
                    //          d(Y)/d(Z) = deactivate(Y)
                    //
                    derivationZ = (*it.pZDeviatioin);

                    //
                    //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
                    //  
                    for(iConvY=0; iConvY<nConvHeight; iConvY++) {
                        varConvYBackup.pIn = it.pIn;
                        varConvYBackup.pInDeviation = it.pInDeviation;
                        varConvYBackup.pWeights = it.pWeights;
                        varConvYBackup.pWeightDerivation = it.pWeightDerivation;
                        for(iConvX=0; iConvX<nConvWidth; iConvX++) {
                            varConvXBackup.pIn = it.pIn;
                            varConvXBackup.pInDeviation = it.pInDeviation;
                            varConvXBackup.pWeights = it.pWeights;
                            varConvXBackup.pWeightDerivation = it.pWeightDerivation;
                            for(iLayer=0; iLayer<nInputLayers; iLayer++) {
                                
                                //
                                // 累计计算权重值
                                //
                                (*it.pInDeviation) += derivationZ * (*it.pWeights);
                                (*it.pWeightDerivation) += derivationZ * (*it.pIn);

                                it.pIn++;
                                it.pInDeviation++;
                                it.pWeights++;
                                it.pWeightDerivation++;
                            }
                            it.pIn = varConvXBackup.pIn + nInputWidthStep;
                            it.pInDeviation = varConvXBackup.pInDeviation + nInputWidthStep;
                            it.pWeights = varConvXBackup.pWeights + nConvWidthStep;
                            it.pWeightDerivation = varConvXBackup.pWeightDerivation + nConvWidthStep;
                        }
                        it.pIn = varConvYBackup.pIn + nInputHeightStep;
                        it.pInDeviation = varConvYBackup.pInDeviation + nInputHeightStep;
                        it.pWeights = varConvYBackup.pWeights + nConvHeightStep;
                        it.pWeightDerivation = varConvYBackup.pWeightDerivation + nConvHeightStep;
                    }

                    //
                    //  偏置的偏导数刚好是输出的偏导数的负数，所以，下降梯度值为(-derivationZ)
                    //
                    (*pBaisArray) -= (-derivationZ) * dLearnRate;

                    it.pIn = varConvBackup.pIn;
                    it.pInDeviation = varConvBackup.pInDeviation;
                    it.pWeights = varConvBackup.pWeights + nConvSize;
                    it.pWeightDerivation = varConvBackup.pWeightDerivation + nConvSize;
                    it.pBais++;
                    it.pZDeviatioin++;
                }

                it.pWeights = varOXBackup.pWeights;
                it.pWeightDerivation = varOXBackup.pWeightDerivation;
                it.pBais = varOXBackup.pBais;
                it.pZDeviatioin = varOXBackup.pZDeviatioin;

                it.pIn += nInputWidthStep;
                it.pInDeviation += nInputWidthStep;
                it.pZDeviatioin += nOutWstep;
            }

            it.pIn = varOYBackup.pIn + nInputHeightStep;
            it.pInDeviation = varOYBackup.pInDeviation + nInputHeightStep;
            it.pZDeviatioin = varOYBackup.pZDeviatioin + nOutHstep;
        }

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInputTensorSize;
        it.pInDeviation = varTBackup.pInDeviation + nInputTensorSize;
        it.pOut = varTBackup.pOut + nOutputTensorSize;
        it.pOutDeviation = varTBackup.pOutDeviation + nOutputTensorSize;
    }

    // 
    // 权重值更新，需要在输入偏导数计算完成后进行，否则，中间会影响输入偏导数值
    //
    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        DVV(pWeightArray, iWeight, nWeights) -= DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate;

        //权重值范围是否需要限制为[-1,1]?
        //if( DVV(pWeightArray, iWeight, nWeights) > 1) {
        //    DVV(pWeightArray, iWeight, nWeights) = 1;
        //}else if( DVV(pWeightArray, iWeight, nWeights) < -1) {
        //    DVV(pWeightArray, iWeight, nWeights) = -1;
        //}
    }

    #ifdef _DEBUG

    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        avgWeight += DVV(pWeightArray,iWeight,nWeights) / nWeights;
        avgDerivation += abs(DVV(pWeightDerivationArray, iWeight, nWeights) * dLearnRate) / nWeights;

        if(maxW < DVV(pWeightArray,iWeight,nWeights)) {
            maxW = DVV(pWeightArray,iWeight,nWeights);
        }
        if(minW > DVV(pWeightArray,iWeight,nWeights)) {
            minW = DVV(pWeightArray,iWeight,nWeights);
        }
    }
    for(int iConv = 0; iConv<nConvs; iConv++) {
        avgBais += abs(DVV(pBaisArray, iConv, nConvs)) / nConvs;
    }
    
    static int t = 0;
    if( (t++ / 2) % 10 == 0) {
        std::cout << "Conv: " << nWeights << " ,Weight: " << minW << " ," << avgWeight <<" ," << maxW <<" , Bais: " << avgBais << ", AvgWD: " << avgDerivation << ", AvgOutD: " << avgOutDerivation << "\n";
    }
    #endif//_DEBUG
    return sCtx.success();
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
            return sCtx.error("卷积网络的输入张量维度，必须大于等于3，其中第一个维度为张量个数，第二三个维度为卷积运算高和宽");
        }

        int nTensor = pInDimSizes[0];
        int nInputHeight = pInDimSizes[1];
        int nInputWidth = pInDimSizes[2];
        if( nInputHeight < m_nConvWidth || nInputWidth < m_nConvWidth ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
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
        if( STensor::createVector<int>(m_spOutDimVector, 4, pOutDimSizes) != sCtx.success() ) {
            return sCtx.error("创建输出张量的维度向量失败");
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
        /*
        int nConvSize = m_nConvWidth*m_nConvHeight*nInputLayers;
        double xWeight = 1.0/nConvSize;
        for(int i=0; i<nWeight; i++) {
            *(pWeights+i) = (rand() % 10000 / 10000.0) * xWeight;
        }*/

        double xWeight = 0.1;//sqrt(1.0/(m_nConvWidth*m_nConvHeight*nInputLayers));
        for(int i=0; i<nWeight; i++) {
            pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
        }

        for(int i=0; i<m_nConvs; i++ ){
            pBais[i] = 0;
        }
    }
    return sCtx.success();
}
