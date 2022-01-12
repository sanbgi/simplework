#include "CRotConvNetwork.h"
#include "math.h"
#include "iostream"

static SCtx sCtx("CRotConvNetwork");
int CRotConvNetwork::createNetwork(int nWidth, int nHeight, int nConvs, double dWidthRotAngle, double dHeightRotAngle, const char* szActivator, SNeuralNetwork& spNetwork) {
    CPointer<CRotConvNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_sizeConv = {
        nConvs,
        nHeight,
        nWidth,
        0
    };
    spConvolution->m_nPadingWidth = 0;
    spConvolution->m_nPadingHeight = 0;
    spConvolution->m_nStrideWidth = 1;
    spConvolution->m_nStrideHeight = 1;
    spConvolution->m_dWRotAngle = dWidthRotAngle;
    spConvolution->m_dHRotAngle = dHeightRotAngle;
    spConvolution->m_pActivator = CActivator::getActivation(szActivator);
    if(spConvolution->m_pActivator == nullptr) {
        return sCtx.error((std::string("不支持的激活函数名: ") + szActivator).c_str());
    }
    if( COptimizer::getOptimizer(nullptr, spConvolution->m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }
    spNetwork.setPtr(spConvolution.getPtr());
    return sCtx.success();
}

int CRotConvNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    if( initNetwork(spInTensor) != sCtx.success() ) {
        return sCtx.error();
    }

    if(spInTensor->getDataSize() != m_nInData) {
        return sCtx.error();
    }

    CBatchSize3D& sizeIn = m_sizeIn;
    CBatchSize3D& sizeOut = m_sizeOut;
    CBatchSize3D& sizeConv = m_sizeConv;
    CBatchSize2D& stepInMove = m_stepInMove;
    CBatchSize2D& stepInConv = m_stepInConv;
    CBatchSize2D& stepOut = m_stepOut;
    CBatchSize2D& stepConv = m_stepConv;
    struct CRotConv {
        int start;
        int width;
        int height;
    };
    CRotConv rConv0 = {
        0,
        m_stepConv.height,
        m_stepConv.width,
    };
    CRotConv rConv90 = {
        (sizeConv.height - 1) * stepConv.height,
        stepConv.width,
        -stepConv.height
    };
    CRotConv rConv180 = {
        (sizeConv.height - 1) * stepConv.height + (sizeConv.width - 1 ) * stepConv.width,
        -stepConv.height,
        -stepConv.width
    };
    CRotConv rConv270 = {
        (sizeConv.width - 1 ) * stepConv.width,
        -stepConv.width,
        stepConv.height
    };

    struct CItOutVariables {
        double* pIn;
        double* pOut;
        double* pWeights;
        double* pBais;
    }it = {
        spInTensor->getDataPtr<double>(),
        m_spOutTensor->getDataPtr<double>(),
        m_spWeights,
        m_spBais
    };

    double dConv, dRotConv, dAngle90, dAngle360, dRatio, dRotRatio[2];
    int iRot, iTensor, iOutY, iOutX, iConv, iConvY, iConvX, iLayer;
    CRotConv* pRotConv;
    CRotConv* pRotConvArray[2];
    CItOutVariables varTBackup, varOYBackup, varOXBackup, varConvBackup, varConvYBackup, varConvXBackup;
    for(iTensor=0; iTensor<sizeIn.batch; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pOut,
        };
        for(iOutY=0; iOutY < sizeOut.height; iOutY++) {
            varOYBackup.pIn = it.pIn;
            varOYBackup.pOut = it.pOut;
            for(iOutX=0; iOutX < sizeOut.width; iOutX++) {
                varOXBackup.pIn = it.pIn;
                varOXBackup.pOut = it.pOut;
                varOXBackup.pWeights = it.pWeights;
                varOXBackup.pBais = it.pBais;

                dAngle360 = fmod(m_dWRotAngle * iOutY + m_dHRotAngle * iOutX, 360);
                dAngle90 = fmod(dAngle360, 90.0);
                switch((int)(dAngle360 / 90.0)) {
                    case 0: // 0 - 90度
                        pRotConvArray[0] = &rConv0;
                        pRotConvArray[1] = &rConv90;
                        break;
                    case 1: // 90 - 180度
                        pRotConvArray[0] = &rConv90;
                        pRotConvArray[1] = &rConv180;
                        break;
                    case 2: // 180 - 270度
                        pRotConvArray[0] = &rConv180;
                        pRotConvArray[1] = &rConv270;
                        break;
                    case 3: // 270 - 360度
                        pRotConvArray[0] = &rConv270;
                        pRotConvArray[1] = &rConv0;
                    break;
                }
                dRotRatio[0] = cos(dAngle90);
                dRotRatio[0] = dRotRatio[0]*dRotRatio[0];
                dRotRatio[1] = sin(dAngle90);
                dRotRatio[1] = dRotRatio[1]*dRotRatio[1];

                for(iConv = 0; iConv < sizeConv.batch; iConv++) {
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pWeights = it.pWeights;

                    dConv = 0;
                    for(iRot=0; iRot<2; iRot++)
                    {
                        pRotConv = pRotConvArray[iRot];
                        it.pWeights = varConvBackup.pWeights + pRotConv->start;
                        it.pIn = varConvBackup.pIn;

                        //初始化卷积结果
                        dRotConv = 0;
                        for( iConvY=0; iConvY<sizeConv.height; iConvY++) {
                            varConvYBackup.pIn = it.pIn;
                            varConvYBackup.pWeights = it.pWeights;
                            for(iConvX=0; iConvX<sizeConv.width; iConvX++) {
                                varConvXBackup.pIn = it.pIn;
                                varConvXBackup.pWeights = it.pWeights;
                                for( int iLayer=0; iLayer<sizeConv.layers; iLayer++) {
                                    //卷积结果为乘积结果求和
                                    dRotConv += (*it.pWeights) * (*it.pIn);
                                    it.pIn++;
                                    it.pWeights++;
                                }
                                it.pIn = varConvXBackup.pIn + stepInConv.width;
                                it.pWeights = varConvXBackup.pWeights + pRotConv->width;
                            }
                            it.pIn = varConvYBackup.pIn + stepInConv.height;
                            it.pWeights = varConvYBackup.pWeights + pRotConv->height;
                        }
                        dConv += dRotConv*dRotRatio[iRot];
                    }

                    //卷积结果减去偏置
                    (*it.pOut) = dConv - (*it.pBais);

                    it.pIn = varConvBackup.pIn;
                    it.pOut++;
                    it.pWeights = varConvBackup.pWeights + stepConv.batch;
                    it.pBais++;
                }
                it.pWeights = varOXBackup.pWeights;
                it.pBais = varOXBackup.pBais;
                it.pIn = varOXBackup.pIn + stepInMove.width;
                it.pOut = varOXBackup.pOut + stepOut.width;
            }
            it.pIn = varOYBackup.pIn + stepInMove.height;
            it.pOut = varOYBackup.pOut + stepOut.height;
        }
        //
        // 激活结果值
        //
        m_pActivator->activate(stepOut.batch, varTBackup.pOut, varTBackup.pOut);
        it.pIn = varTBackup.pIn + stepInMove.batch;
        it.pOut = varTBackup.pOut + stepOut.batch;
    }

    m_spInTensor = spInTensor;
    spOutTensor = m_spOutTensor;
    return sCtx.success();
}

int CRotConvNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spInTensor;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    CBatchSize3D& sizeIn = m_sizeIn;
    CBatchSize3D& sizeOut = m_sizeOut;
    CBatchSize3D& sizeConv = m_sizeConv;
    CBatchSize2D& stepInMove = m_stepInMove;
    CBatchSize2D& stepInConv = m_stepInConv;
    CBatchSize2D& stepOut = m_stepOut;
    CBatchSize2D& stepConv = m_stepConv;
    struct CRotConv {
        int start;
        int width;
        int height;
    };
    CRotConv rConv0 = {
        0,
        m_stepConv.height,
        m_stepConv.width,
    };
    CRotConv rConv90 = {
        (sizeConv.height - 1) * stepConv.height,
        stepConv.width,
        -stepConv.height
    };
    CRotConv rConv180 = {
        (sizeConv.height - 1) * stepConv.height + (sizeConv.width - 1 ) * stepConv.width,
        -stepConv.height,
        -stepConv.width
    };
    CRotConv rConv270 = {
        (sizeConv.width - 1 ) * stepConv.width,
        -stepConv.width,
        stepConv.height
    };

    int nConvs = sizeConv.batch;
    int nWeights = stepConv.batch * sizeConv.batch;
    double* pWeightDerivationArray = m_spOptimizer->getDeviationPtr(nWeights+nConvs);
    double* pBaisDerivationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(double)*(nWeights+nConvs));

    struct CItOutVariables {
        double* pIn;
        double* pInDeviation;
        double* pOut;
        double* pOutDeviation;
        double* pWeights;
        double* pWeightDevivation;
        double* pBaisDeviation;
        double* pZDeviatioin;
    }it = {
        spInTensor->getDataPtr<double>(),
        spInDeviation->getDataPtr<double>(),
        spOutTensor->getDataPtr<double>(),
        spOutDeviation->getDataPtr<double>(),
        m_spWeights,
        pWeightDerivationArray,
        pBaisDerivationArray
    };
    double derivationZ;
    double pDerivationZ[stepOut.batch];
    double dAngle90, dAngle360, dRatio, dRotRatio[2];
    CRotConv* pRotConv;
    CRotConv* pRotConvArray[2];
    int iRot, iTensor, iOutY, iOutX, iConv, iConvY, iConvX, iLayer;
    CItOutVariables varTBackup, varOYBackup, varOXBackup, varConvBackup, varConvYBackup, varConvXBackup;
    for(iTensor=0; iTensor<sizeIn.batch; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pInDeviation,
            it.pOut,
            it.pOutDeviation
        };

        m_pActivator->deactivate(stepOut.batch, it.pOut, it.pOutDeviation, pDerivationZ);
        it.pZDeviatioin = pDerivationZ;
        for(iOutY=0; iOutY < sizeOut.height; iOutY++) {
            varOYBackup.pIn = it.pIn;
            varOYBackup.pInDeviation = it.pInDeviation;
            varOYBackup.pZDeviatioin = it.pZDeviatioin;
            for(iOutX=0; iOutX < sizeOut.width; iOutX++) {
                varOXBackup.pWeights = it.pWeights;
                varOXBackup.pWeightDevivation = it.pWeightDevivation;
                varOXBackup.pBaisDeviation = it.pBaisDeviation;
                varOXBackup.pZDeviatioin = it.pZDeviatioin;

                dAngle360 = fmod(m_dWRotAngle * iOutY + m_dHRotAngle * iOutX, 360);
                dAngle90 = fmod(dAngle360, 90.0);
                switch(((int)(dAngle360 / 90.0))%4) {
                    case 0: // 0 - 90度
                        pRotConvArray[0] = &rConv0;
                        pRotConvArray[1] = &rConv90;
                        break;
                    case 1: // 90 - 180度
                        pRotConvArray[0] = &rConv90;
                        pRotConvArray[1] = &rConv180;
                        break;
                    case 2: // 180 - 270度
                        pRotConvArray[0] = &rConv180;
                        pRotConvArray[1] = &rConv270;
                        break;
                    case 3: // 270 - 360度
                        pRotConvArray[0] = &rConv270;
                        pRotConvArray[1] = &rConv0;
                    break;
                }
                dRotRatio[0] = cos(dAngle90);
                dRotRatio[0] = dRotRatio[0]*dRotRatio[0];
                dRotRatio[1] = sin(dAngle90);
                dRotRatio[1] = dRotRatio[1]*dRotRatio[1];

                for(iConv = 0; iConv < nConvs; iConv++) {
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pInDeviation = it.pInDeviation;
                    varConvBackup.pWeights = it.pWeights;
                    varConvBackup.pWeightDevivation = it.pWeightDevivation;

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

                    for(iRot=0; iRot<2; iRot++)
                    {
                        pRotConv = pRotConvArray[iRot];
                        dRatio = dRotRatio[iRot];
                        it.pIn = varConvBackup.pIn;
                        it.pInDeviation = varConvBackup.pInDeviation;
                        it.pWeights = varConvBackup.pWeights + pRotConv->start;
                        it.pWeightDevivation = varConvBackup.pWeightDevivation + pRotConv->start;

                        //
                        //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
                        //  
                        for(iConvY=0; iConvY<sizeConv.height; iConvY++) {
                            varConvYBackup.pIn = it.pIn;
                            varConvYBackup.pInDeviation = it.pInDeviation;
                            varConvYBackup.pWeights = it.pWeights;
                            varConvYBackup.pWeightDevivation = it.pWeightDevivation;
                            for(iConvX=0; iConvX<sizeConv.width; iConvX++) {
                                varConvXBackup.pIn = it.pIn;
                                varConvXBackup.pInDeviation = it.pInDeviation;
                                varConvXBackup.pWeights = it.pWeights;
                                varConvXBackup.pWeightDevivation = it.pWeightDevivation;
                                for(iLayer=0; iLayer<sizeConv.layers; iLayer++) {
                                    
                                    //
                                    // 累计计算权重值
                                    //
                                    (*it.pInDeviation) += derivationZ * (*it.pWeights) * dRatio;
                                    (*it.pWeightDevivation) += derivationZ * (*it.pIn) * dRatio;

                                    it.pIn++;
                                    it.pInDeviation++;
                                    it.pWeights++;
                                    it.pWeightDevivation++;
                                }
                                it.pIn = varConvXBackup.pIn + stepInConv.width;
                                it.pInDeviation = varConvXBackup.pInDeviation + stepInConv.width;
                                it.pWeights = varConvXBackup.pWeights + pRotConv->width;
                                it.pWeightDevivation = varConvXBackup.pWeightDevivation + pRotConv->width;
                            }
                            it.pIn = varConvYBackup.pIn + stepInConv.height;
                            it.pInDeviation = varConvYBackup.pInDeviation + stepInConv.height;
                            it.pWeights = varConvYBackup.pWeights + pRotConv->height;
                            it.pWeightDevivation = varConvYBackup.pWeightDevivation + pRotConv->height;
                        }
                    }

                    //
                    //  偏置的偏导数刚好是输出的偏导数的负数，所以，下降梯度值为(-derivationZ)
                    //
                    (*it.pBaisDeviation) += (-derivationZ);

                    it.pIn = varConvBackup.pIn;
                    it.pInDeviation = varConvBackup.pInDeviation;
                    it.pWeights = varConvBackup.pWeights + stepConv.batch;
                    it.pWeightDevivation = varConvBackup.pWeightDevivation + stepConv.batch;
                    it.pBaisDeviation++;
                    it.pZDeviatioin++;
                }

                it.pWeights = varOXBackup.pWeights;
                it.pWeightDevivation = varOXBackup.pWeightDevivation;
                it.pBaisDeviation = varOXBackup.pBaisDeviation;
                it.pZDeviatioin = varOXBackup.pZDeviatioin;

                it.pIn += stepInMove.width;
                it.pInDeviation += stepInMove.width;
                it.pZDeviatioin += stepOut.width;
            }

            it.pIn = varOYBackup.pIn + stepInMove.height;
            it.pInDeviation = varOYBackup.pInDeviation + stepInMove.height;
            it.pZDeviatioin = varOYBackup.pZDeviatioin + stepOut.height;
        }

        //  更新迭代参数
        it.pIn = varTBackup.pIn + stepInMove.batch;
        it.pInDeviation = varTBackup.pInDeviation + stepInMove.batch;
        it.pOut = varTBackup.pOut + stepOut.batch;
        it.pOutDeviation = varTBackup.pOutDeviation + stepOut.batch;
    }

    //
    // 用优化器优化偏差值
    //
    m_spOptimizer->updateDeviation(sizeIn.batch);

    // 
    // 权重值更新，需要在输入偏导数计算完成后进行，否则，中间会影响输入偏导数值
    //
    double* pWeightArray = m_spWeights;
    for(int iWeight=0;iWeight<nWeights; iWeight++) {
        DVV(pWeightArray, iWeight, nWeights) -= DVV(pWeightDerivationArray, iWeight, nWeights);
    }
    double* pBaisArray = m_spBais;
    for(int iBais=0; iBais<nConvs; iBais++) {
        DVV(pBaisArray, iBais, nConvs) -= DVV(pBaisDerivationArray, iBais, nConvs);
    }

    /*
    #ifdef _DEBUG
    {
        double avgWeight = 0;
        double maxW = -100000;
        double minW = 100000;
        double avgDerivation = 0;
        double avgBais = 0;
        for(int iWeight=0;iWeight<nWeights; iWeight++) {
            avgWeight += DVV(pWeightArray,iWeight,nWeights) / nWeights;
            avgDerivation += abs(DVV(pWeightDerivationArray, iWeight, nWeights)) / nWeights;

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
            std::cout << "Conv: " << nWeights << " ,Weight: " << minW << " ," << avgWeight <<" ," << maxW <<" , Bais: " << avgBais << ", AvgWD: " << avgDerivation << "\n";
        }
    }
    #endif//_DEBUG
    */

    return sCtx.success();
}

int CRotConvNetwork::initNetwork(const STensor& spInTensor) {
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
        int nInHeight = pInDimSizes[1];
        int nInWidth = pInDimSizes[2];
        if( nInHeight < m_sizeConv.height || nInWidth < m_sizeConv.width ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        int nInLayers = 1;
        for( int i=3; i<nInDims; i++ ) {
            nInLayers *= pInDimSizes[i];
        }
        m_nInData = nTensor*nInHeight*nInWidth*nInLayers;
        m_sizeConv.layers = nInLayers;
        m_sizeIn = {
            nTensor,
            nInHeight,
            nInWidth,
            nInLayers
        };

        m_sizeOut = {
            m_sizeIn.batch,
            (m_sizeIn.height - m_sizeConv.height) / m_nStrideHeight + 1,
            (m_sizeIn.width - m_sizeConv.width) / m_nStrideWidth + 1,
            m_sizeConv.batch
        };

        m_stepInConv = {
            m_sizeIn.height * m_sizeIn.width * nInLayers,
            m_sizeIn.width * nInLayers,
            nInLayers
        };

        m_stepInMove = {  
            m_stepInConv.batch,
            m_stepInConv.height * m_nStrideHeight,
            m_stepInConv.width * m_nStrideWidth
        };

        m_stepOut = {
            m_sizeOut.height * m_sizeOut.width * m_sizeConv.batch,
            m_sizeOut.width * m_sizeConv.batch,
            m_sizeConv.batch
        };

        m_stepConv = {
            m_sizeConv.height * m_sizeConv.width * nInLayers,
            m_sizeConv.width * nInLayers,
            nInLayers
        };

        if( STensor::createVector<int>(m_spOutDimVector, 4, (int*)&m_sizeOut) != sCtx.success() ) {
            return sCtx.error("创建输出张量的维度向量失败");
        }

        if( STensor::createTensor<double>(m_spOutTensor, m_spOutDimVector, m_sizeIn.batch * m_stepOut.batch) != sCtx.success() ){
            return sCtx.error("创建输出张量失败");
        }

        int nWeights = m_stepConv.batch * m_sizeConv.batch;
        int nConvs = m_sizeConv.batch;
        double* pWeights = new double[nWeights];
        double* pBais = new double[nConvs];
        m_spWeights.take(pWeights, [](double* pWeights){
            delete[] pWeights;
        });
        m_spBais.take(pBais, [](double* pBais){
            delete[] pBais;
        });

        //
        // TODO：权重初始化采用什么策略？nadam优化算法下，即便所有权重初始值为零，仍然不影响迭代
        //
        double xWeight = 0.1;//sqrt(1.0/(m_nConvWidth*m_nConvHeight*nInLayers));
        for(int i=0; i<nWeights; i++) {
            //pWeights[i] = 0;
            pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
        }

        for(int i=0; i<m_sizeConv.batch; i++ ){
            pBais[i] = 0;
        }
    }
    return sCtx.success();
}
