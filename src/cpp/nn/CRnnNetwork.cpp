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

        int nWeights = m_nCells*(nInputCells+m_nCells);
        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nWeights, m_nCells);
        }else if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nWeights, m_nCells);
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

    if( SVectorSolver::getSolver(idType, m_spSolver) != sCtx.success() ) {
        return sCtx.error((std::string("创建向量求解器失败")).c_str());
    }

    m_spBatchInDeviation.release();
    m_nBatchs = nBatchs;
    m_nGroups = nGroups;
    m_nInputSize = nInputSize;
    return sCtx.success();
}


template<typename Q> int CRnnNetwork::initWeightT(int nWeights, int nCells) {
    Q* pWeights = new Q[nWeights];
    Q* pState = new Q[nCells];
    Q* pBais = new Q[nCells];
    m_spWeights.take((char*)pWeights, [](char* pWeights){
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
    for(int i=0; i<nWeights; i++) {
        pWeights[i] = -xWeight + CUtils::rand() * xWeight * 2;
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
    int nWeights = nInputCells*(nInputCells*nCells);
    bool bKeepGroup = m_bKeepGroup;

    //
    // 丢弃数组长度为输出数组长度，值为同等位置输出是否需要抛弃。所以，根据抛弃比率，随机计算抛弃状态
    //
    /*
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
    */

    //
    //执行内部状态值的指针，用于在最后一步，将状态计算结果(网络输出)保存下来
    //
    Q* pState = (Q*)(void*)m_spState;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pWeight;
        Q* pState;
        Q* pBais;
    }varBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spInternalBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spWeights,
        pState,
        (Q*)(void*)m_spBais
    };
    varBackup = it;

    IVectorSolver* pSolver = m_spSolver.getPtr();
    Q joinedVec[nCells+nInputCells];
    PVector outVar = {
        m_nCells,
        nullptr
    };
    PVectorArray inVars_Join = {2,{
        { nInputCells, nullptr },
        { nCells, it.pState }
    }};
    PVectorArray inVars_Multiply = {2,{
        { nInputCells + nCells, joinedVec },
        { nWeights, it.pWeight }
    }};
    PVectorArray inVars_Bais = {2,{
        { nCells, nullptr },
        { nCells, it.pBais }
    }};
    for(int iTensor=0; iTensor<nTensors; iTensor++) {
        //
        // 输入向量和状态向量连接
        //
        outVar.data = joinedVec;
        outVar.size = nCells + nInputCells;
        inVars_Join.data[0].data = it.pIn;
        inVars_Join.data[1].data = it.pState;
        if( pSolver->join(inVars_Join, outVar) != sCtx.success() ) {
            return sCtx.error("连接输入和状态向量异常");
        }

        //
        // 权重矩阵相乘
        //
        outVar.size = nCells;
        outVar.data = it.pOut;
        if( pSolver->multiply(inVars_Multiply, outVar) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }

        //
        // 加上偏置
        //
        inVars_Bais.data[0].data = it.pOut;
        outVar.size = nCells;
        outVar.data = it.pOut;
        if( pSolver->add(inVars_Bais, outVar) != sCtx.success() ) {
            return sCtx.error("偏置计算异常");
        }

        //
        // 激活
        //
        m_pActivator->activate(nCells, it.pOut, it.pOut);

        //
        //  更新状态值
        //
        if(iTensor == nTensors-1) {
            memcpy(pState, it.pOut, nOutputSize);
        }else{
            it.pState = it.pOut;
        }
        it.pIn += nInputCells;
        it.pOut += nCells;
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

    int nBatchs = m_nBatchs;
    int nGroups = m_nGroups;
    int nTensor = nBatchs * nGroups;
    int nInputCells = m_nInputCells;
    int nCells = m_nCells;
    int nJoinedInput = nCells + nInputCells;
    bool bKeepGroup = m_bKeepGroup;

    int nWeights = nCells * (nInputCells + nCells);
    int nVariables = nWeights + nCells;
    Q* pWeightDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nVariables);
    Q* pBaisDeviationArray = pWeightDerivationArray+nWeights;
    memset(pWeightDerivationArray, 0 ,sizeof(Q)*(nVariables));
    Q pZDeviationArray[nCells], pStateDeviationArray[nCells];
    Q pJoinDeviationArray[nJoinedInput], pJoinIn[nJoinedInput];
    Q* pJoinInState = pJoinIn+nInputCells;
    Q* pInDeviation = spBatchInDeviation->getDataPtr<Q>();
    memset(pInDeviation, 0, sizeof(Q)*nTensor*nInputCells);

    //
    //  注意：输出偏差（pOutDeviation）相对特殊，如果一组对应一个输出，则输出
    //  偏差对应的是这一组的最后输出偏差，而不是中间值，具体中间值的输出偏差，会
    //  使用上一次从输出偏差计算出来的状态输入偏差。
    //
    Q* pState;
    struct CItOutVariables {
        Q* pIn;
        Q* pInDeviation;
        Q* pOut;
        Q* pOutDeviation;
        Q* pWeight;
        Q* pWeightDeviation;
        Q* pBaisDeviation;
        Q* pZDeviatioin;
        Q* pState;
    }it;

    it.pIn = spBatchIn->getDataPtr<Q>() + (nTensor-1)*nInputCells;
    it.pInDeviation = pInDeviation + (nTensor-1)*nInputCells;
    it.pOut = spBatchOut->getDataPtr<Q>() + (nTensor-1)*nCells;
    it.pOutDeviation = spBatchOutDeviation->getDataPtr<Q>() + (nTensor-1)*nCells;//这个地方小心
    it.pWeight = (Q*)(void*)m_spWeights;
    it.pWeightDeviation = pWeightDerivationArray;
    it.pBaisDeviation = pBaisDeviationArray;
    it.pState = (Q*)(void*)m_spState;

    IVectorSolver* pSolver = m_spSolver.getPtr();
    PVector outDVar = {
        nCells,
        pZDeviationArray
    };
    PVectorArray inVars_DMultiply = {2, {
        { nCells+nInputCells, pJoinIn },
        { nWeights, it.pWeight }
    }};
    PVectorArray inDVars_DMultiply = {2, {
        { nCells+nInputCells, pJoinDeviationArray },
        { nWeights, it.pWeightDeviation }
    }};

    int iTensor, iGroupOut;
    int nGroupOut = m_bKeepGroup ? 1 : nGroups;
    for(iTensor=0, iGroupOut=0; iTensor<nTensor; iTensor++) {
        //
        //  非一组中最后一个输出时（非倒序的第一个），其偏差等于上一此计算的状态输入偏差，其它情况为实际输出偏差（由外层传入）
        //
        if(iGroupOut>0) {
            pSolver->copy(nCells, pStateDeviationArray, pJoinDeviationArray + nInputCells);
            it.pOutDeviation = pStateDeviationArray;
        }

        //
        // 初始化组合后的输入向量，以及输入偏导向量
        //
        pSolver->zero(nJoinedInput, pJoinDeviationArray);
        pSolver->copy(nInputCells, pJoinIn, it.pIn);
        if(iTensor == 0) {
            pSolver->copy(nCells, pJoinInState, it.pState);
        }
        if(iTensor == nTensor-1) {
            pSolver->copy(nCells, pJoinInState, it.pOut-nCells);
        }else{
            pSolver->zero(nCells, pJoinInState);
        }

        //
        //  计算目标函数相对于Y值的偏导数
        //
        m_pActivator->deactivate(m_nCells, it.pOut, it.pOutDeviation, pZDeviationArray);
        it.pZDeviatioin = pZDeviationArray;

        //
        //  偏置的偏导数更新
        //
        pSolver->add(nCells, pZDeviationArray, it.pBaisDeviation);

        //
        // 计算权重及输入的偏导数
        //
        if( pSolver->multiplyAccDeviation(outDVar, inDVars_DMultiply, inVars_DMultiply) != sCtx.success() ) {
            return sCtx.error("偏导数计算错误");
        }

        //
        // 输入的偏导更新
        //
        pSolver->copy(nInputCells, it.pInDeviation, pJoinDeviationArray);

        iGroupOut++;
        if(iGroupOut == nGroupOut) {
            it.pOutDeviation -= nCells;
            iGroupOut = 0;
        }

        //  更新迭代参数
        it.pIn -= nInputCells;
        it.pInDeviation -= nInputCells;
        it.pOut -= nCells;
    }

    m_spOptimizer->updateDeviation(nTensor);
    pSolver->del(nWeights, it.pWeight, pWeightDerivationArray);
    pSolver->del(nCells, (Q*)(void*)m_spBais, pBaisDeviationArray);
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
        ar.visitTaker("weights", nBytes*m_nCells*(m_nInputCells+m_nCells), m_spWeights);
        ar.visitTaker("state", nBytes*m_nCells, m_spState);
        ar.visitTaker("bais", nBytes*m_nCells, m_spBais);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CRnnNetwork, CRnnNetwork::__getClassKey())