#include "CDenseNetwork.h"
#include "CType.h"
#include <math.h>
#include <iostream>
#include <time.h>

static SCtx sCtx("CDenseNetwork");
int CDenseNetwork::createNetwork(int nCells, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork) {
    CPointer<CDenseNetwork> spDense;
    CObject::createObject(spDense);
    spDense->m_nCells = nCells;
    spDense->m_dDropoutRate = dDropoutRate;
    if( szActivator!=nullptr )
        spDense->m_strActivator = szActivator;
    spNetwork.setPtr(spDense.getPtr());
    return sCtx.success();
}

template<typename Q> int CDenseNetwork::initWeightT(int nWeights, int nCells) {
    Q* pWeights = new Q[nWeights];
    Q* pBais = new Q[nCells];
    m_spWeights.take((char*)pWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spBais.take((char*)pBais, [](char* pBais){
        delete[] (Q*)pBais;
    });

    //
    // 初始化权重值，从[0-1]均匀分布（注意不是随机值）
    //
    /*
    Q xWeight = 1.0/nInputCells;
    for(int i=0; i<nWeight; i++) {
        *(pWeights+i) = (rand() % 10000 / 10000.0) * xWeight;
    }*/
    Q xWeight = 0.1;//sqrt(1.0/nInputCells);
    for(int i=0; i<nWeights; i++) {
        pWeights[i] = -xWeight + (rand() % 10000 / 10000.0) * xWeight * 2;
    }
    for(int i=0; i<nCells; i++ ){
        pBais[i] = 0;
    }

    return 0;
}

int CDenseNetwork::prepareNetwork(const STensor& spBatchIn) {
    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn->getDataSize();
    unsigned int idType = spBatchIn.type();
    if( nInputSize == m_nInputCells * m_nBatchs  && m_idDataType == idType) {
        return sCtx.success();
    }

    //
    // 计算详细维度尺寸
    //
    int nBatchs = 0;
    int nInputCells = 0;
    {
        //
        // 检查输入张量维度
        //
        STensor& spInDimTensor = spBatchIn->getDimVector();
        int nInputDims = spInDimTensor->getDataSize();
        if(nInputDims < 2) {
            return sCtx.error("输入张量维度需要大于1，其中第一个维度是批量张量个数");
        }

        //
        // 计算输入参数
        //
        int* pDimSizes = spInDimTensor->getDataPtr<int>();
        nBatchs = pDimSizes[0];
        nInputCells = pDimSizes[1];
        for( int i=2; i<nInputDims; i++) {
            nInputCells *= pDimSizes[i];
        }
        if(nBatchs * nInputCells != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 判断是否需要重新初始化网络
    //
    if(m_nInputCells==0) {
        //
        //
        //
        if( m_nCells <= 0 || nInputCells <= 0 ) {
            return sCtx.error("不允许全连接网络细胞数或输入细胞数为零");
        }

        int nWeights = m_nCells*nInputCells;

        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nWeights, m_nCells);
        }else if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nWeights, m_nCells);
        }else{
            return sCtx.error("不支持的数据类型");
        }

        m_nBatchs = 0; //通过这个值的设置，实现之后的运行时参数必须重新初始化
        m_nInputCells = nInputCells;
        m_idDataType = idType;
    }else{
        if(m_nInputCells != nInputCells) {
            //
            // TODO: 重新初始化权值?(如果重新初始化，则已有权值将被清空，需要小心)
            //
            return sCtx.error("当前输入的参数，与神经网络需要的参数不符");
        }

        if(m_idDataType != idType) {
            //
            // TODO：权值类型改变？
            //
            return sCtx.error("输入张量的类型与初始化时不一致");
        }
    }
    
    //
    //  如果抛弃比率大于2个神经元，则dropout机制有效
    //
    if(m_dDropoutRate > 2.0 / m_nCells) {
        m_spDropout.take(new bool[m_nCells], [](bool* ptr){
            delete[] ptr;
        });
    }

    //
    // 检查细胞数量是否合法
    //
    int pOutDimSizes[2] = { nBatchs, m_nCells };
    STensor spOutDimVector;
    if( int errCode = STensor::createVector(spOutDimVector, 2, pOutDimSizes) != sCtx.success() ) {
        return sCtx.error(errCode, "创建神经网络输出张量维度向量失败");
    }

    if( int errCode = STensor::createTensor(m_spBatchOut, spOutDimVector, idType, nBatchs * m_nCells) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输出张量失败");
    }

    m_pActivator = CActivator::getActivation(idType, m_strActivator.c_str());
    if(m_pActivator == nullptr) {
        return sCtx.error((std::string("不支持的激活函数名: ") + m_strActivator).c_str());
    }

    if( COptimizer::getOptimizer(m_strOptimizer.c_str(), idType, m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    m_spBatchInDeviation.release();
    m_nBatchs = nBatchs;
    return sCtx.success();
}

template<typename Q> int CDenseNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut)
{
    int nBatchs = m_nBatchs;
    int nOutCells = m_nCells;
    int nWeights = m_nInputCells*m_nCells;
    int nInputCells = m_nInputCells;

    bool* pDropout = m_spDropout, *pDrop;
    int nDropout = 0;
    if(pDropout) {
        srand(time(nullptr));
        bool* ptr = pDropout;
        for(int i=0; i<nOutCells; i++) {
            bool bDropout = (rand() * 1.0 / RAND_MAX) < m_dDropoutRate;
            if(bDropout) {
                nDropout++;
            }
            *ptr = bDropout;
            ptr++;
        }
    }
    m_nEvalDropout = nDropout;
    Q xDropScale = nOutCells / (Q)(nOutCells-nDropout);

    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pWeight;
        Q* pBais;
    }varTBackup, varOBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        (Q*)(void*)m_spBais
    };

    Q dOut;
    int iTensor, iOutput, iInput;
    for(iTensor=0; iTensor<nBatchs; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pOut,
            it.pWeight,
            it.pBais,
        };

        //
        //  调整权重
        //
        if(nDropout > 1) {
            pDrop = pDropout;
            for(iOutput=0; iOutput<nOutCells; iOutput++) {
                varOBackup = {
                    it.pIn,
                    it.pOut,
                    it.pWeight,
                };

                if(*pDrop) {
                    (*it.pOut) = 0;
                }else{
                    dOut = 0;
                    for(iInput=0; iInput<nInputCells; iInput++ ) {
                        dOut += (*it.pWeight) * (*it.pIn);
                        it.pIn++;
                        it.pWeight++;
                    }
                    (*it.pOut) = (dOut - (*it.pBais))*xDropScale;
                }

                //  更新迭代参数
                pDrop++;
                it.pOut++;
                it.pBais++;
                it.pIn = varOBackup.pIn;
                it.pWeight = varOBackup.pWeight + nInputCells;
            }
        }else{
            for(iOutput=0; iOutput<nOutCells; iOutput++) {

                varOBackup = {
                    it.pIn,
                    it.pOut,
                    it.pWeight,
                };

                dOut = 0;
                for(iInput=0; iInput<nInputCells; iInput++ ) {
                    dOut += (*it.pWeight) * (*it.pIn);
                    it.pIn++;
                    it.pWeight++;
                }

                (*it.pOut) = dOut - (*it.pBais);

                //  更新迭代参数
                it.pOut++;
                it.pBais++;
                it.pIn = varOBackup.pIn;
                it.pWeight = varOBackup.pWeight + nInputCells;
            }
        }

        m_pActivator->activate(nOutCells, varTBackup.pOut, varTBackup.pOut);

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInputCells;
        it.pOut = varTBackup.pOut + nOutCells;
        it.pWeight = varTBackup.pWeight;
        it.pBais = varTBackup.pBais;
    }

    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CDenseNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( int errCode = prepareNetwork(spBatchIn) != sCtx.success() ) {
        return errCode;
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }

    return sCtx.error("数据类型不支持");
}

template<typename Q> int CDenseNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if( !m_spBatchInDeviation ) {
        if( int errCode = STensor::createTensor(m_spBatchInDeviation, m_spBatchIn->getDimVector(), m_idDataType, m_spBatchIn->getDataSize()) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输入偏差张量失败");
        }
    }
    spBatchIn = m_spBatchIn;
    spBatchInDeviation = m_spBatchInDeviation;

    bool* pDropout = m_spDropout;
    int nDropout = m_nEvalDropout;
    Q xScaleDropout = (m_nCells - nDropout) / (Q) m_nCells;
    Q* pDev;
    bool* pDrop;

    int nBatchs = m_nBatchs;
    int nInputCells = m_nInputCells;
    int nCells = m_nCells;
    int nWeights = m_nCells * m_nInputCells;
    
    Q* pWeightDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nWeights+m_nCells);
    Q* pBaisDeviationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(Q)*(nWeights+m_nCells));

    struct CItOutVariables {
        Q* pIn;
        Q* pInDeviation;
        Q* pOut;
        Q* pOutDeviation;
        Q* pWeight;
        Q* pWeightDeviation;
        Q* pBaisDeviation;
        Q* pZDeviatioin;
    }varTBackup, varOBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        spBatchInDeviation->getDataPtr<Q>(),
        spBatchOut->getDataPtr<Q>(),
        spBatchOutDeviation->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        pWeightDerivationArray,
        pBaisDeviationArray
    };
    memset(it.pInDeviation, 0, sizeof(Q)*nBatchs*nInputCells);

    Q pZDerivationArray[m_nCells], deviationZ;
    int iTensor, iOutput, iInput;
    for(iTensor=0; iTensor<nBatchs; iTensor++) {
        varTBackup = {
            it.pIn,
            it.pInDeviation,
            it.pOut,
            it.pOutDeviation,
            it.pWeight,
            it.pWeightDeviation,
            it.pBaisDeviation,
        };

        //
        //  计算目标函数相对于Y值的偏导数
        //
        m_pActivator->deactivate(m_nCells, it.pOut, it.pOutDeviation, pZDerivationArray);
        it.pZDeviatioin = pZDerivationArray;

        if(nDropout > 0) {
            pDrop = pDropout;
            pDev = pZDerivationArray;
            for(iOutput=0; iOutput<m_nCells; iOutput++) {
                if(*pDrop) {
                    *pDev = 0;
                }else{
                    *pDev *= xScaleDropout;
                }
                pDrop++, pDev++;
            }
        }

        //
        //  调整权重
        //
        for(iOutput=0; iOutput<nCells; iOutput++) {
            varOBackup = {
                it.pIn,
                it.pInDeviation,
                it.pOut,
                it.pOutDeviation,
                it.pWeight,
                it.pWeightDeviation,
            };

            //
            //  计算目标函数对当前输出值的偏导数
            //      X = 输入
            //      Y = 权重*X-偏置
            //      F = activation(Y)
            //      delta = F - F0(目标)
            //      E = delta*delta/2 目标函数
            //      deviationZ = d(E)/d(Y) = d(E)/d(delta) * d(delta)/d(F) * d(F)/d(Y)
            //      其中：
            //          d(E)/d(delta) = pOutDeviation[iOutput]
            //          d(delta)/d(F) = 1
            //          d(F)/d(Y) = deactivate(Y)
            //
            deviationZ = *(it.pZDeviatioin);
            if(deviationZ > 1.0e-16 || deviationZ < -1.0e-16) {
                //
                // 更新权重，权重值的偏导数=输出值偏导数*数入值
                //
                for(iInput=0; iInput<nInputCells; iInput++ ) {

                    //
                    // 输入对实际目标的偏差值，反向传递给上一层，其实就是相对于输入的偏导数
                    //
                    // 注意：这里是否要乘以学习率？
                    //      如果乘以学习率后，相当于向前传递的不是偏导数，而是偏导数 * 学习率，与现有神经网络BP算法不一致
                    //      如果不乘以学习率，相当于直接向前传递偏导数，与现有神经网络BP算法一致，但含义上有点奇怪   
                    //
                    //pInDeviation[iInput] += deviationZ * pWeights[iWeight] * dLearnRate;
                    (*it.pInDeviation) += deviationZ * (*it.pWeight);
                    (*it.pWeightDeviation) += deviationZ * (*it.pIn);

                    it.pIn++;
                    it.pInDeviation++;
                    it.pWeight++;
                    it.pWeightDeviation++;
                }

                //
                // 更新偏移，偏移值的偏导数= (-输出值偏导数)，因为具体值为wx-b=y
                //
                *it.pBaisDeviation += (-deviationZ);
            }

            //  更新迭代参数
            it.pBaisDeviation++;
            it.pZDeviatioin++;
            it.pIn = varOBackup.pIn;
            it.pInDeviation = varOBackup.pInDeviation;
            it.pWeight = varOBackup.pWeight + nInputCells;
            it.pWeightDeviation = varOBackup.pWeightDeviation + nInputCells;
        }

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInputCells;
        it.pInDeviation = varTBackup.pInDeviation + nInputCells;
        it.pOut = varTBackup.pOut + nCells;
        it.pOutDeviation = varTBackup.pOutDeviation + nCells;
        it.pWeight = varTBackup.pWeight;
        it.pWeightDeviation = varTBackup.pWeightDeviation;
        it.pBaisDeviation = varTBackup.pBaisDeviation;
    }

    m_spOptimizer->updateDeviation(nBatchs);

    Q* pWeightArray = (Q*)(void*)m_spWeights;
    Q* pWeightEnd = pWeightArray+nWeights;
    while(pWeightArray != pWeightEnd) {
        *pWeightArray -= *pWeightDerivationArray;
        pWeightArray++, pWeightDerivationArray++;
    }
    Q* pBaisArray = (Q*)(void*)m_spBais;
    Q* pBaisEnd = pBaisArray+m_nCells;
    while(pBaisArray != pBaisEnd) {
        *pBaisArray -= *pBaisDeviationArray;
        pBaisArray++, pBaisDeviationArray++;
    }
    return sCtx.success();
}

int CDenseNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    if(spBatchOut.type() != m_idDataType) {
        return sCtx.error("数据类型错误");
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return learnT<double>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return learnT<float>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

int CDenseNetwork::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("nCells", m_nCells);
    ar.visit("dDropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    ar.visitString("optimizer", m_strOptimizer);

    //运行参数
    ar.visit("nInputCells", m_nInputCells);
    ar.visit("dataType", m_idDataType);
    if(m_nInputCells) {
        int nBytes = CType::getTypeBytes(m_idDataType);
        ar.visitTaker("weights", nBytes*m_nCells*m_nInputCells, m_spWeights);
        ar.visitTaker("bais", nBytes*m_nCells, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseNetwork, CDenseNetwork::__getClassKey())