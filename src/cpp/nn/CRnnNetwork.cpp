#include "CRnnNetwork.h"
#include "CType.h"
#include "CUtils.h"
#include <math.h>

static SCtx sCtx("CRnnNetwork");
int CRnnNetwork::createNetwork(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork) {
    CPointer<CRnnNetwork> spRnn;
    CObject::createObject(spRnn);
    spRnn->m_nCells = nCells;
    spRnn->m_bKeepGroup = bKeepGroup;
    spRnn->m_dDropoutRate = dDropoutRate;
    if( szActivator!=nullptr )
        spRnn->m_strActivator = szActivator;
    spNetwork.setPtr(spRnn.getPtr());
    return sCtx.success();
}

int CRnnNetwork::prepareNetwork(const STensor& spBatchIn) {
    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn->getDataSize();
    unsigned int idType = spBatchIn.type();
    if( nInputSize == m_nInputSize  && m_idDataType == idType) {
        return sCtx.success();
    }

    //
    // 计算详细维度尺寸
    //
    int nBatchs = 0;
    int nGroups = 0;
    int nInputCells = 0;
    {
        //
        // 检查输入张量维度
        //
        STensor& spInDimTensor = spBatchIn->getDimVector();
        int nInputDims = spInDimTensor->getDataSize();
        if(nInputDims < 3) {
            return sCtx.error("输入张量维度需要大于2，其中第一个维度是批量数据组数，第二个维度是分组数据个数");
        }

        //
        // 计算输入参数
        //
        int* pDimSizes = spInDimTensor->getDataPtr<int>();
        nBatchs = pDimSizes[0];
        nGroups = pDimSizes[1];
        nInputCells = pDimSizes[2];
        for( int i=3; i<nInputDims; i++) {
            nInputCells *= pDimSizes[i];
        }
        if(nBatchs * nGroups * nInputCells != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 判断是否需要重新初始化网络
    //
    if(m_nInputCells == 0) {
        //
        //
        //
        if( m_nCells <= 0 || nInputCells <= 0 ) {
            return sCtx.error("不允许循环神经网络细胞数或输入细胞数为零");
        }

        int nUWeights = m_nCells*nInputCells;
        int nVWeights = m_nCells*m_nCells;

        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nUWeights, nVWeights, m_nCells);
        }else if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nUWeights, nVWeights, m_nCells);
        }else{
            return sCtx.error("不支持的数据类型");
        }

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
    // 创建状态缓存张量和输出结果张量，注意：
    //  1，如果需要保留分组输出，则输出张量和状态张量一致
    //  2，如果不需要输出，则单独创建输出张量
    //
    int pOutDimSizes[3] = { nBatchs, nGroups, m_nCells };
    STensor spOutDimVector;
    if( int errCode = STensor::createVector(spOutDimVector, 3, pOutDimSizes) != sCtx.success() ) {
        return sCtx.error(errCode, "创建神经网络输出张量维度向量失败");
    }

    if( int errCode = STensor::createTensor(m_spInternalBatchOut, spOutDimVector, idType, nBatchs * nGroups * m_nCells) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输出张量失败");
    }

    if(!m_bKeepGroup) {
        int pOutDimSizes[2] = { nBatchs, m_nCells };
        STensor spOutDimVector;
        if( int errCode = STensor::createVector(spOutDimVector, 2, pOutDimSizes) != sCtx.success() ) {
            return sCtx.error(errCode, "创建神经网络输出张量维度向量失败");
        }

        if( int errCode = STensor::createTensor(m_spBatchOut, spOutDimVector, idType, nBatchs * m_nCells) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输出张量失败");
        }
    }else{
        m_spBatchOut = m_spInternalBatchOut;
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
    m_nGroups = nGroups;
    m_nInputSize = nInputSize;
    return sCtx.success();
}


template<typename Q> int CRnnNetwork::initWeightT(int nUWeights, int nVWeights, int nCells) {
    Q* pUWeights = new Q[nUWeights];
    Q* pVWeights = new Q[nVWeights];
    Q* pState = new Q[nCells];
    Q* pBais = new Q[nCells];
    m_spUWeights.take((char*)pUWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spVWeights.take((char*)pVWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spState.take((char*)pState, [](char* pState){
        delete[] (Q*)pState;
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
        *(pWeights+i) = CUtils::rand() * xWeight;
    }*/
    Q xWeight = 0.1;//sqrt(1.0/nInputCells);
    for(int i=0; i<nUWeights; i++) {
        pUWeights[i] = -xWeight + CUtils::rand() * xWeight * 2;
    }
    for(int i=0; i<nVWeights; i++) {
        pVWeights[i] = -xWeight + CUtils::rand() * xWeight * 2;
    }
    for(int i=0; i<nCells; i++ ){
        pBais[i] = 0;
        pState[i] = 0;
    }

    return 0;
}

template<typename Q> int CRnnNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut)
{
    int nBatchs = m_nBatchs;
    int nGroups = m_nGroups;
    int nTensors = nBatchs * nGroups;
    int nCells = m_nCells;
    int nOutputSize = m_nCells*sizeof(Q);
    int nInputCells = m_nInputCells;
    int nUWeights = nInputCells*nCells;
    int nVWeights = nCells*nCells;
    bool bKeepGroup = m_bKeepGroup;

    //
    // 丢弃数组长度为输出数组长度，值为同等位置输出是否需要抛弃。所以，根据抛弃比率，随机计算抛弃状态
    //
    bool* pDropout = m_spDropout, *pDrop;
    int nDropout = 0;
    if(pDropout) {
        bool* ptr = pDropout;
        for(int i=0; i<nCells; i++) {
            bool bDropout = CUtils::rand() < m_dDropoutRate;
            if(bDropout) {
                nDropout++;
            }
            *ptr = bDropout;
            ptr++;
        }
    }
    m_nEvalDropout = nDropout;
    Q xDropScale = nCells / (Q)(nCells-nDropout);

    //
    //执行内部状态值的指针，用于在最后一步，将状态计算结果(网络输出)保存下来
    //
    Q* pState = (Q*)(void*)m_spState;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pUWeight;
        Q* pVWeight;
        Q* pState;
        Q* pBais;
    }varBackup, varTBackup, varOBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spInternalBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spUWeights,
        (Q*)(void*)m_spVWeights,
        pState,
        (Q*)(void*)m_spBais
    };

    Q dUOut, dVOut;
    int iTensor, iOutput, iInput;
    varBackup = it;
    for(iTensor=0; iTensor<nTensors; iTensor++) {
        varTBackup.pIn = it.pIn;
        varTBackup.pOut = it.pOut;
        varTBackup.pUWeight = it.pUWeight;
        varTBackup.pVWeight = it.pVWeight;
        varTBackup.pBais = it.pBais;

        //
        // 计算输出值
        //
        if(nDropout > 1) {
            pDrop = pDropout;
            for(iOutput=0; iOutput<nCells; iOutput++) {
                varOBackup.pIn = it.pIn;
                varOBackup.pState = it.pState;
                varOBackup.pUWeight = it.pUWeight;
                varOBackup.pVWeight = it.pVWeight;
                if(*pDrop) {
                    (*it.pOut) = 0;
                }else{
                    dUOut = 0;
                    for(iInput=0; iInput<nInputCells; iInput++ ) {
                        dUOut += (*it.pUWeight) * (*it.pIn);
                        it.pIn++;
                        it.pUWeight++;
                    }

                    dVOut = 0;
                    for(iInput=0; iInput<nCells; iInput++ ) {
                        dVOut += (*it.pVWeight) * (*it.pState);
                        it.pState++;
                        it.pVWeight++;
                    }
                    
                    (*it.pOut) = (dUOut + dVOut - (*it.pBais))*xDropScale;
                }

                //  更新迭代参数
                pDrop++;
                it.pOut++;
                it.pBais++;
                it.pIn = varOBackup.pIn;
                it.pState = varOBackup.pState;
                it.pUWeight = varOBackup.pUWeight + nInputCells;
                it.pVWeight = varOBackup.pVWeight + nCells;
            }
        }else{
            for(iOutput=0; iOutput<nCells; iOutput++) {
                varOBackup.pIn = it.pIn;
                varOBackup.pState = it.pState;
                varOBackup.pUWeight = it.pUWeight;
                varOBackup.pVWeight = it.pVWeight;

                dUOut = 0;
                for(iInput=0; iInput<nInputCells; iInput++ ) {
                    dUOut += (*it.pUWeight) * (*it.pIn);
                    it.pIn++;
                    it.pUWeight++;
                }

                dVOut = 0;
                for(iInput=0; iInput<nCells; iInput++ ) {
                    dVOut += (*it.pVWeight) * (*it.pState);
                    it.pState++;
                    it.pVWeight++;
                }
                
                (*it.pOut) = dUOut + dVOut - (*it.pBais);

                //  更新迭代参数
                it.pOut++;
                it.pBais++;
                it.pIn = varOBackup.pIn;
                it.pState = varOBackup.pState;
                it.pUWeight = varOBackup.pUWeight + nInputCells;
                it.pVWeight = varOBackup.pVWeight + nCells;
            }
        }

        m_pActivator->activate(nCells, varTBackup.pOut, varTBackup.pOut);

        //  最后一步更新状态值
        if(iTensor == nTensors-1) {
            memcpy(pState, varTBackup.pOut, nOutputSize);
        }else{
            it.pState = varTBackup.pOut;
        }
        it.pIn = varTBackup.pIn + nInputCells;
        it.pOut = varTBackup.pOut + nCells;
        it.pUWeight = varTBackup.pUWeight;
        it.pVWeight = varTBackup.pVWeight;
        it.pBais = varTBackup.pBais;
    }

    m_spBatchIn = spBatchIn;
    m_spInternalBatchOut.updateVer();
    m_nInVer = spBatchIn.ver();
    if(bKeepGroup) {
        spBatchOut = m_spInternalBatchOut;
    }else{
        int nBatchStep = nGroups * nCells;
        Q* pOut = varBackup.pOut + nBatchStep - nCells;
        Q* pGroupOut = m_spBatchOut->getDataPtr<Q>();
        for(int iBatch=0; iBatch<nBatchs; iBatch++) {
            memcpy(pGroupOut, pOut, nOutputSize);
            pOut += nBatchStep;
            pGroupOut += nCells;
        }
        m_spBatchOut.updateVer();
        spBatchOut = m_spBatchOut;
    }
    m_nOutVer = spBatchOut.ver();
    return sCtx.success();
}

int CRnnNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( int errCode = prepareNetwork(spBatchIn) != sCtx.success() ) {
        return errCode;
    }

    if(spBatchIn.getPtr() == m_spBatchIn.getPtr() && spBatchIn.ver() == m_nInVer ) {
        spBatchOut = m_spInternalBatchOut;
        return sCtx.success();
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return evalT<double>(spBatchIn, spBatchOut);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return evalT<float>(spBatchIn, spBatchOut);
    }

    return sCtx.error("数据类型不支持");
}

template<typename Q> int CRnnNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if( !m_spBatchInDeviation ) {
        if( int errCode = STensor::createTensor(m_spBatchInDeviation, m_spBatchIn->getDimVector(), m_idDataType, m_spBatchIn->getDataSize()) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输入偏差张量失败");
        }
    }else{
        m_spBatchInDeviation.updateVer();
    }
    spBatchIn = m_spBatchIn;
    spBatchInDeviation = m_spBatchInDeviation;

    bool* pDropout = m_spDropout;
    int nDropout = m_nEvalDropout;
    Q xScaleDropout = (m_nCells - nDropout) / (Q) m_nCells;
    Q* pDev;
    bool* pDrop;

    int nBatchs = m_nBatchs;
    int nGroups = m_nGroups;
    int nTensor = nBatchs * nGroups;
    int nInputCells = m_nInputCells;
    int nCells = m_nCells;
    int nOutputSize = m_nCells*sizeof(Q);
    bool bKeepGroup = m_bKeepGroup;

    int nUWeights = nCells * nInputCells;
    int nVWeights = nCells * nCells;
    int nVariables = nUWeights + nVWeights + nCells;
    Q* pUWeightDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nVariables);
    Q* pVWeightDerivationArray = pUWeightDerivationArray+nUWeights;
    Q* pBaisDeviationArray = pVWeightDerivationArray+nVWeights;
    memset(pUWeightDerivationArray, 0 ,sizeof(Q)*(nVariables));

    //
    //  注意：输出偏差（pOutDeviation）相对特殊，如果一组对应一个输出，则输出
    //  偏差对应的是这一组的最后输出偏差，而不是中间值，具体中间值的输出偏差，会
    //  使用上一次从输出偏差计算出来的状态输入偏差。
    //
    Q* pState;
    struct CItOutVariables {
        Q* pIn;
        Q* pInDeviation;
        Q* pStateDeviation;
        Q* pOut;
        Q* pOutDeviation;
        Q* pUWeight;
        Q* pVWeight;
        Q* pUWeightDeviation;
        Q* pVWeightDeviation;
        Q* pBaisDeviation;
        Q* pZDeviatioin;
    }varTBackup, varOBackup, it;
    Q* pInDeviation = spBatchIn->getDataPtr<Q>();
    it.pIn = spBatchIn->getDataPtr<Q>() + (nTensor-1)*nInputCells;
    it.pInDeviation = pInDeviation + (nTensor-1)*nInputCells;
    it.pOut = spBatchOut->getDataPtr<Q>() + (nTensor-1)*nCells;
    it.pOutDeviation = spBatchOutDeviation->getDataPtr<Q>() + (nTensor-1)*nCells;//这个地方小心
    it.pUWeight = (Q*)(void*)m_spUWeights;
    it.pVWeight = (Q*)(void*)m_spVWeights;
    it.pUWeightDeviation = pUWeightDerivationArray;
    it.pVWeightDeviation = pVWeightDerivationArray;
    it.pBaisDeviation = pBaisDeviationArray;
    memset(pInDeviation, 0, sizeof(Q)*nTensor*nInputCells);

    Q pZDeviationArray[nCells], pStateDeviationArray[nCells], pTmpStateDeviationArray[nCells], deviationZ;
    int iTensor, iOutput, iInput, iGroupOut;
    int nGroupOut = m_bKeepGroup ? 1 : nGroups;
    for(iTensor=0, iGroupOut=0; iTensor<nTensor; iTensor++) {
        //备份迭代器参数
        varTBackup = it;

        //
        //  非一组中最后一个输出时（非倒序的第一个），其偏差等于上一此计算的状态输入偏差，其它情况为实际输出偏差（由外层传入）
        //
        if(iGroupOut>0) {
            memcpy(pStateDeviationArray, pTmpStateDeviationArray, nOutputSize);
            it.pOutDeviation = pStateDeviationArray;
        }
        memset(pTmpStateDeviationArray,0, nOutputSize);
        it.pStateDeviation = pTmpStateDeviationArray;

        //
        //  计算目标函数相对于Y值的偏导数
        //
        m_pActivator->deactivate(m_nCells, it.pOut, it.pOutDeviation, pZDeviationArray);
        it.pZDeviatioin = pZDeviationArray;

        if(nDropout > 0) {
            pDrop = pDropout;
            pDev = pZDeviationArray;
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
            varOBackup.pIn = it.pIn;
            varOBackup.pInDeviation = it.pInDeviation;
            varOBackup.pStateDeviation = it.pStateDeviation;
            varOBackup.pUWeight = it.pUWeight;
            varOBackup.pUWeightDeviation = it.pUWeightDeviation,
            varOBackup.pVWeight = it.pVWeight;
            varOBackup.pVWeightDeviation = it.pVWeightDeviation,
                
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
                    (*it.pInDeviation) += deviationZ * (*it.pUWeight);
                    (*it.pUWeightDeviation) += deviationZ * (*it.pIn);

                    it.pIn++;
                    it.pInDeviation++;
                    it.pUWeight++;
                    it.pUWeightDeviation++;
                }

                //如果不是第一个输入，则还需要继续将偏导向前一个状态输入传递
                if(iTensor != nTensor-1) {
                    pState = it.pOut - nCells;
                    for(iInput=0; iInput<nCells; iInput++ ) {
                        (*it.pStateDeviation) += deviationZ * (*it.pVWeight);
                        (*it.pVWeightDeviation) += deviationZ * (*pState);

                        pState++;
                        it.pStateDeviation++;
                        it.pVWeight++;
                        it.pVWeightDeviation++;
                    }
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
            it.pStateDeviation = varOBackup.pStateDeviation;
            it.pUWeight = varOBackup.pUWeight + nInputCells;
            it.pUWeightDeviation = varOBackup.pUWeightDeviation + nInputCells;
            it.pVWeight = varOBackup.pVWeight + nCells;
            it.pVWeightDeviation = varOBackup.pVWeightDeviation + nCells;
        }

        iGroupOut++;
        if(iGroupOut == nGroupOut) {
            it.pOutDeviation = varTBackup.pOutDeviation - nCells;
            iGroupOut = 0;
        }

        //  更新迭代参数
        it.pIn -= nInputCells;
        it.pInDeviation -= nInputCells;
        it.pOut = varTBackup.pOut - nCells;
        it.pUWeight = varTBackup.pUWeight;
        it.pUWeightDeviation = varTBackup.pUWeightDeviation;
        it.pVWeight = varTBackup.pVWeight;
        it.pVWeightDeviation = varTBackup.pVWeightDeviation;
        it.pBaisDeviation = varTBackup.pBaisDeviation;
    }

    m_spOptimizer->updateDeviation(nTensor);

    Q* pUWeightArray = (Q*)(void*)m_spUWeights;
    Q* pUWeightEnd = pUWeightArray+nUWeights;
    while(pUWeightArray != pUWeightEnd) {
        *pUWeightArray -= *pUWeightDerivationArray;
        pUWeightArray++, pUWeightDerivationArray++;
    }
    Q* pVWeightArray = (Q*)(void*)m_spVWeights;
    Q* pVWeightEnd = pVWeightArray+nVWeights;
    while(pVWeightArray != pVWeightEnd) {
        *pVWeightArray -= *pVWeightDerivationArray;
        pVWeightArray++, pVWeightDerivationArray++;
    }
    Q* pBaisArray = (Q*)(void*)m_spBais;
    Q* pBaisEnd = pBaisArray+m_nCells;
    while(pBaisArray != pBaisEnd) {
        *pBaisArray -= *pBaisDeviationArray;
        pBaisArray++, pBaisDeviationArray++;
    }
    return sCtx.success();
}

int CRnnNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {

    if(spBatchOut.getPtr() != m_spBatchOut.getPtr() || spBatchOut.ver() != m_nOutVer) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }
    
    if(spBatchOutDeviation.type() != m_idDataType || spBatchOutDeviation.size() != spBatchOut.size() ) {
        return sCtx.error("偏差数据类型或尺寸错误");
    }

    if(m_idDataType == CBasicData<double>::getStaticType()) {
        return learnT<double>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }else
    if(m_idDataType == CBasicData<float>::getStaticType()) {
        return learnT<float>(spBatchOut, spBatchOutDeviation, spBatchIn, spBatchInDeviation);
    }
    return sCtx.error("数据类型不支持");
}

int CRnnNetwork::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visit("keepGroup", m_bKeepGroup);
    ar.visit("dropoutRate", m_dDropoutRate);
    ar.visitString("activator", m_strActivator);
    ar.visitString("optimizer", m_strOptimizer);

    //运行参数
    ar.visit("inputCells", m_nInputCells);
    ar.visit("dataType", m_idDataType);
    if(m_nInputCells) {
        int nBytes = CType::getTypeBytes(m_idDataType);
        ar.visitTaker("uweights", nBytes*m_nCells*m_nInputCells, m_spUWeights);
        ar.visitTaker("vweights", nBytes*m_nCells*m_nCells, m_spVWeights);
        ar.visitTaker("state", nBytes*m_nCells, m_spState);
        ar.visitTaker("bais", nBytes*m_nCells, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CRnnNetwork, CRnnNetwork::__getClassKey())