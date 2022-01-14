#include "CConvolutionNetwork.h"
#include "CType.h"
#include "math.h"


static SCtx sCtx("CConvolutionNetwork");
int CConvolutionNetwork::createNetwork(int nWidth, int nHeight, int nConvs, const char* szActivator, SNnNetwork& spNetwork) {

    if(nWidth < 1 || nHeight < 1 || nConvs < 1) {
        return sCtx.error("卷积核参数错误");
    }

    CPointer<CConvolutionNetwork> spConvolution;
    CObject::createObject(spConvolution);
    spConvolution->m_sizeConv = {
        nConvs,
        nHeight,
        nWidth,
        0
    };
    spConvolution->m_nStrideWidth = 1;
    spConvolution->m_nStrideHeight = 1;
    if( szActivator!=nullptr )
        spConvolution->m_strActivator = szActivator;
    spNetwork.setPtr(spConvolution.getPtr());
    return sCtx.success();
}


int CConvolutionNetwork::prepareNetwork(const STensor& spBatchIn) {
    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn.size();
    unsigned int idType = spBatchIn.type();
    if( nInputSize == m_nInputSize && m_idDataType == idType) {
        return sCtx.success();
    }

    //
    // 计算参数
    //
    int nBatchs, nInputCells, nInputWidth, nInputHeight, nLayers;
    {
        STensor& spInDimVector = spBatchIn->getDimVector();
        int nInDims = spInDimVector->getDataSize();
        int* pInDimSizes = spInDimVector->getDataPtr<int>();

        //
        // 维度小于3的张量，无法进行卷积运算
        //
        if(nInDims < 3) {
            return sCtx.error("卷积网络的输入张量维度，必须大于等于3，其中第一个维度为张量个数，第二三个维度为卷积运算高和宽");
        }

        nBatchs = pInDimSizes[0];
        nInputHeight = pInDimSizes[1];
        nInputWidth = pInDimSizes[2];
        if( nInputHeight < m_sizeConv.height || nInputWidth < m_sizeConv.width ) {
            return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
        }

        nLayers = 1;
        for( int i=3; i<nInDims; i++ ) {
            nLayers *= pInDimSizes[i];
        }
        nInputCells = nInputWidth*nInputHeight*nLayers;
        if(nBatchs * nInputCells != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 判断是否需要初始化网络
    //
    if(m_nLayers == 0) {

        if( m_sizeConv.batch <= 0 || m_sizeConv.width < 1 || m_sizeConv.height < 1 ) {
            return sCtx.error("卷积核参数错误");
        }
        
        m_pActivator = CActivator::getActivation(idType, m_strActivator.c_str());
        if(m_pActivator == nullptr) {
            return sCtx.error((std::string("不支持的激活函数名: ") + m_strActivator).c_str());
        }

        if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, m_spOptimizer) != sCtx.success()) {
            return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
        }

        int nConvs = m_sizeConv.batch;
        int nWeights = nLayers * m_sizeConv.width * m_sizeConv.height * m_sizeConv.batch;
        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nWeights, nConvs);
        }else if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nWeights, nConvs);
        }else{
            return sCtx.error("不支持的数据类型");
        }

        m_sizeIn.batch = 0; //通过这个值的设置，实现之后的运行时参数必须重新初始化
        m_nLayers = nLayers;
        m_idDataType = idType;
    }else {
        if( m_nLayers != nLayers || m_idDataType != idType ) {
            return sCtx.error("输出张量的层数与初始化时不一致");
        }
    }

    //
    // 判断是否需要初始化运行时参数
    //
    if( m_nInputSize != nInputSize ) {
        
        m_sizeConv.layers = nLayers;
        m_sizeIn = {
            nBatchs,
            nInputHeight,
            nInputWidth,
            nLayers
        };

        m_sizeOut = {
            m_sizeIn.batch,
            (m_sizeIn.height - m_sizeConv.height) / m_nStrideHeight + 1,
            (m_sizeIn.width - m_sizeConv.width) / m_nStrideWidth + 1,
            m_sizeConv.batch
        };

        m_stepInConv = {
            m_sizeIn.height * m_sizeIn.width * nLayers,
            m_sizeIn.width * nLayers,
            nLayers
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
            m_sizeConv.height * m_sizeConv.width * nLayers,
            m_sizeConv.width * nLayers,
            nLayers
        };

        if( STensor::createVector<int>(m_spOutDimVector, 4, (int*)&m_sizeOut) != sCtx.success() ) {
            return sCtx.error("创建输出张量的维度向量失败");
        }

        if( STensor::createTensor(m_spBatchOut, m_spOutDimVector, idType, m_sizeIn.batch * m_stepOut.batch) != sCtx.success() ){
            return sCtx.error("创建输出张量失败");
        }

        m_nInputSize = nInputSize;
    }
    return sCtx.success();
}

template<typename Q> int CConvolutionNetwork::initWeightT(int nWeights, int nConvs) {
    Q* pWeights = new Q[nWeights];
    Q* pBais = new Q[nConvs];
    m_spWeights.take( (char*)pWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spBais.take((char*)pBais, [](char* pBais){
        delete[] (Q*)pBais;
    });

    //
    // TODO：权重初始化采用什么策略？nadam优化算法下，即便所有权重初始值为零，仍然不影响迭代
    //
    Q xWeight = 0.1;//sqrt(1.0/(m_nConvWidth*m_nConvHeight*nInLayers));
    for(int i=0; i<nWeights; i++) {
        //pWeights[i] = 0;
        pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
    }

    for(int i=0; i<m_sizeConv.batch; i++ ){
        pBais[i] = 0;
    }

    return 0;
}

template<typename Q> int CConvolutionNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {

    CBatchSize3D& sizeIn = m_sizeIn;
    CBatchSize3D& sizeOut = m_sizeOut;
    CBatchSize3D& sizeConv = m_sizeConv;
    CBatchSize2D& stepInMove = m_stepInMove;
    CBatchSize2D& stepInConv = m_stepInConv;
    CBatchSize2D& stepOut = m_stepOut;
    CBatchSize2D& stepConv = m_stepConv;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pWeights;
        Q* pBais;
        int index;
    }itVars0, itVars1, itVars2, itVars3, itVars4, itVars5, itVars6, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        (Q*)(void*)m_spBais,
    };

    Q dConv;
    for(itVars0.index=0; itVars0.index < sizeIn.batch; itVars0.index++) {
        itVars0.pIn = it.pIn;
        itVars0.pOut = it.pOut;
        for(itVars1.index=0; itVars1.index < sizeOut.height; itVars1.index++) {
            itVars1.pIn = it.pIn;
            itVars1.pOut = it.pOut;
            for(itVars2.index=0; itVars2.index < sizeOut.width; itVars2.index++) {
                itVars2.pIn = it.pIn;
                itVars2.pOut = it.pOut;
                itVars2.pWeights = it.pWeights;
                itVars2.pBais = it.pBais;
                for(itVars3.index = 0; itVars3.index < sizeConv.batch; itVars3.index++) {
                    itVars3.pIn = it.pIn;
                    itVars3.pWeights = it.pWeights;

                    //初始化卷积结果
                    dConv = 0;
                    for( itVars4.index=0; itVars4.index<sizeConv.height; itVars4.index++) {
                        itVars4.pIn = it.pIn;
                        itVars4.pWeights = it.pWeights;
                        for(itVars5.index=0; itVars5.index<sizeConv.width; itVars5.index++) {
                            itVars5.pIn = it.pIn;
                            itVars5.pWeights = it.pWeights;
                            for(itVars6.index=0; itVars6.index<sizeConv.layers; itVars6.index++) {
                                //卷积结果为乘积结果求和
                                dConv += (*it.pWeights) * (*it.pIn);
                                it.pIn++;
                                it.pWeights++;
                            }
                            it.pIn = itVars5.pIn + stepInConv.width;
                            it.pWeights = itVars5.pWeights + stepConv.width;
                        }
                        it.pIn = itVars4.pIn + stepInConv.height;
                        it.pWeights = itVars4.pWeights + stepConv.height;
                    }
                    //卷积结果减去偏置
                    (*it.pOut) = dConv - (*it.pBais);

                    it.pIn = itVars3.pIn;
                    it.pOut++;
                    it.pWeights = itVars3.pWeights + stepConv.batch;
                    it.pBais++;
                }
                it.pWeights = itVars2.pWeights;
                it.pBais = itVars2.pBais;
                it.pIn = itVars2.pIn + stepInMove.width;
                it.pOut = itVars2.pOut + stepOut.width;
            }
            it.pIn = itVars1.pIn + stepInMove.height;
            it.pOut = itVars1.pOut + stepOut.height;
        }

        //
        // 激活结果值
        //
        m_pActivator->activate(stepOut.batch, itVars0.pOut, itVars0.pOut);

        it.pIn = itVars0.pIn + stepInMove.batch;
        it.pOut = itVars0.pOut + stepOut.batch;
    }

    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CConvolutionNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( prepareNetwork(spBatchIn) != sCtx.success() ) {
        return sCtx.error();
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }

    return sCtx.error("数据类型不支持");
}


/*
    #include <sys/time.h>
    using namespace std;
    timeval t1, t2, t3; 
    gettimeofday(&t1, nullptr);
    gettimeofday(&t2, nullptr);
    gettimeofday(&t3, nullptr);
    long dt1 = (t2.tv_sec-t1.tv_sec)*1000000+(t2.tv_usec-t1.tv_usec);
    long dt2 = (t3.tv_sec-t2.tv_sec)*1000000+(t3.tv_usec-t2.tv_usec);
*/
template<typename Q> int CConvolutionNetwork::learnT(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {

    spBatchIn = m_spBatchIn;
    if( int errCode = STensor::createTensor<Q>(spInDeviation, spBatchIn->getDimVector(), spBatchIn->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    CBatchSize3D& sizeIn = m_sizeIn;
    CBatchSize3D& sizeOut = m_sizeOut;
    CBatchSize3D& sizeConv = m_sizeConv;
    CBatchSize2D& stepInMove = m_stepInMove;
    CBatchSize2D& stepInConv = m_stepInConv;
    CBatchSize2D& stepOut = m_stepOut;
    CBatchSize2D& stepConv = m_stepConv;

    int nConvs = sizeConv.batch;
    int nWeights = stepConv.batch * sizeConv.batch;
    Q* pWeightDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nWeights+nConvs);
    Q* pBaisDeviationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(Q)*(nWeights+nConvs));

    struct CItOutVariables {
        Q* pIn;
        Q* pInDeviation;
        Q* pOut;
        Q* pOutDeviation;
        Q* pWeights;
        Q* pWeightDevivation;
        Q* pBaisDeviation;
        Q* pZDeviatioin;
        int index;
    }it = {
        spBatchIn->getDataPtr<Q>(),
        spInDeviation->getDataPtr<Q>(),
        spBatchOut->getDataPtr<Q>(),
        spOutDeviation->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        pWeightDerivationArray,
        pBaisDeviationArray
    }, itVars, itVars0,itVars1,itVars2,itVars3,itVars4,itVars5,itVars6;

    Q deviationZ;
    Q pDeviationZArray[stepOut.batch];

    itVars = it;
    for(itVars0.index=0; itVars0.index<sizeIn.batch; itVars0.index++) {
        itVars0.pIn = it.pIn;
        itVars0.pInDeviation = it.pInDeviation;
        itVars0.pOut = it.pOut;
        itVars0.pOutDeviation = it.pOutDeviation; 

        m_pActivator->deactivate(stepOut.batch, it.pOut, it.pOutDeviation, pDeviationZArray);
        it.pZDeviatioin = pDeviationZArray;

        for(itVars1.index=0; itVars1.index < sizeOut.height; itVars1.index++) {
            itVars1.pIn = it.pIn;
            itVars1.pInDeviation = it.pInDeviation;
            itVars1.pZDeviatioin = it.pZDeviatioin;
            for(itVars2.index=0; itVars2.index < sizeOut.width; itVars2.index++) {
                itVars2.pWeights = it.pWeights;
                itVars2.pWeightDevivation = it.pWeightDevivation;
                itVars2.pBaisDeviation = it.pBaisDeviation;
                itVars2.pZDeviatioin = it.pZDeviatioin;
                for(itVars3.index = 0; itVars3.index < nConvs; itVars3.index++) {
                    itVars3.pIn = it.pIn;
                    itVars3.pInDeviation = it.pInDeviation;
                    itVars3.pWeights = it.pWeights;
                    itVars3.pWeightDevivation = it.pWeightDevivation;

                    //
                    //  计算目标函数对当前输出值的偏导数
                    //      X = 输入
                    //      Z = 权重*X-偏置
                    //      Y = activation(Z)
                    //      delta = 目标 - Y
                    //      E = delta*delta/2 目标函数
                    //      deviationZ = d(E) / d(Z) = d(E)/d(delta) * d(delta)/d(Y) * d(F)/d(Z)
                    //      其中：
                    //          d(E)/d(delta) = pOutDeviation[iOutput]
                    //          d(delta)/d(Y) = 1
                    //          d(Y)/d(Z) = deactivate(Y)
                    //
                    deviationZ = (*it.pZDeviatioin);
                    if(deviationZ > 1.0e-16 || deviationZ < -1.0e-16) 
                    {
                        //
                        //  计算每一个输出对输入及权重的偏导数，并以此来调整权重及输入
                        //  
                        for(itVars4.index=0; itVars4.index<sizeConv.height; itVars4.index++) {
                            itVars4.pIn = it.pIn;
                            itVars4.pInDeviation = it.pInDeviation;
                            itVars4.pWeights = it.pWeights;
                            itVars4.pWeightDevivation = it.pWeightDevivation;
                            for(itVars5.index=0; itVars5.index<sizeConv.width; itVars5.index++) {
                                itVars5.pIn = it.pIn;
                                itVars5.pInDeviation = it.pInDeviation;
                                itVars5.pWeights = it.pWeights;
                                itVars5.pWeightDevivation = it.pWeightDevivation;
                                for(itVars6.index=0; itVars6.index<sizeConv.layers; itVars6.index++) {
                                    
                                    //
                                    // 累计计算权重值
                                    //
                                    (*it.pInDeviation) += deviationZ * (*it.pWeights);
                                    (*it.pWeightDevivation) += deviationZ * (*it.pIn);

                                    it.pIn++;
                                    it.pInDeviation++;
                                    it.pWeights++;
                                    it.pWeightDevivation++;
                                }
                                it.pIn = itVars5.pIn + stepInConv.width;
                                it.pInDeviation = itVars5.pInDeviation + stepInConv.width;
                                it.pWeights = itVars5.pWeights + stepConv.width;
                                it.pWeightDevivation = itVars5.pWeightDevivation + stepConv.width;
                            }

                            it.pIn = itVars4.pIn + stepInConv.height;
                            it.pInDeviation = itVars4.pInDeviation + stepInConv.height;
                            it.pWeights = itVars4.pWeights + stepConv.height;
                            it.pWeightDevivation = itVars4.pWeightDevivation + stepConv.height;
                        }

                        //
                        //  偏置的偏导数刚好是输出的偏导数的负数，所以，下降梯度值为(-deviationZ)
                        //
                        (*it.pBaisDeviation) += (-deviationZ);
                    }

                    it.pIn = itVars3.pIn;
                    it.pInDeviation = itVars3.pInDeviation;
                    it.pWeights = itVars3.pWeights + stepConv.batch;
                    it.pWeightDevivation = itVars3.pWeightDevivation + stepConv.batch;
                    it.pBaisDeviation++;
                    it.pZDeviatioin++;
                }

                it.pWeights = itVars2.pWeights;
                it.pWeightDevivation = itVars2.pWeightDevivation;
                it.pBaisDeviation = itVars2.pBaisDeviation;
                it.pZDeviatioin = itVars2.pZDeviatioin;

                it.pIn += stepInMove.width;
                it.pInDeviation += stepInMove.width;
                it.pZDeviatioin += stepOut.width;
            }

            it.pIn = itVars1.pIn + stepInMove.height;
            it.pInDeviation = itVars1.pInDeviation + stepInMove.height;
            it.pZDeviatioin = itVars1.pZDeviatioin + stepOut.height;
        }

        //  更新迭代参数
        it.pIn = itVars0.pIn + stepInMove.batch;
        it.pInDeviation = itVars0.pInDeviation + stepInMove.batch;
        it.pOut = itVars0.pOut + stepOut.batch;
        it.pOutDeviation = itVars0.pOutDeviation + stepOut.batch;
    }
    it = itVars;
    
    //
    // 用优化器优化偏差值
    //
    m_spOptimizer->updateDeviation(sizeIn.batch);

    // 
    // 权重值更新，需要在输入偏导数计算完成后进行，否则，中间会影响输入偏导数值
    //
    Q* pWeightEnd = it.pWeights+nWeights;
    while(it.pWeights != pWeightEnd) {
        *it.pWeights -= *it.pWeightDevivation;
        it.pWeights++, it.pWeightDevivation++;
    }
    Q* pBais = (Q*)(void*)m_spBais;
    Q* pBaisEnd = pBais+nConvs;
    while(pBais != pBaisEnd) {
        *pBais -= *it.pBaisDeviation;
        pBais++, it.pBaisDeviation++;
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

int CConvolutionNetwork::learn(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    if(spOutDeviation.type() != m_idDataType) {
        return sCtx.error("数据类型错误");
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return learnT<double>(spBatchOut, spOutDeviation, spBatchIn, spInDeviation);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return learnT<float>(spBatchOut, spOutDeviation, spBatchIn, spInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

int CConvolutionNetwork::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("convs", m_sizeConv.batch);
    ar.visit("dropout", m_sizeConv.batch);
    ar.visit("width", m_sizeConv.width);
    ar.visit("height", m_sizeConv.height);
    ar.visit("strideWidth", m_nStrideWidth);
    ar.visit("strideHeight", m_nStrideHeight);
    ar.visitString("activator", m_strActivator);
    ar.visitString("optimizer", m_strOptimizer);
    ar.visitString("padding", m_strPadding);

    //运行参数
    ar.visit("inputLayers", m_nLayers);
    ar.visit("dataType", m_idDataType);
    if(m_nLayers) {
        int nBytes = CType::getTypeBytes(m_idDataType);
        ar.visitTaker("weights", nBytes * m_nLayers * m_sizeConv.width * m_sizeConv.height * m_sizeConv.batch, m_spWeights);
        ar.visitTaker("bais", nBytes * m_sizeConv.batch, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CConvolutionNetwork, CConvolutionNetwork::__getClassKey())