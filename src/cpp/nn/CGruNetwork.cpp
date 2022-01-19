#include "CGruNetwork.h"
#include "CType.h"
#include "CUtils.h"
#include <math.h>

static SCtx sCtx("CGruNetwork");
int CGruNetwork::createNetwork(int nCells, bool bKeepGroup, double dDropoutRate, const char* szActivator, SNnNetwork& spNetwork) {
    CPointer<CGruNetwork> spGru;
    CObject::createObject(spGru);
    spGru->m_nCells = nCells;
    spGru->m_bKeepGroup = bKeepGroup;
    spGru->m_dDropoutRate = dDropoutRate;
    if( szActivator!=nullptr )
        spGru->m_strActivator = szActivator;
    spNetwork.setPtr(spGru.getPtr());
    return sCtx.success();
}

int CGruNetwork::prepareNetwork(const STensor& spBatchIn) {
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

    m_pSigmodActivator = CActivator::getActivation(idType, "sigmod");
    if(m_pSigmodActivator == nullptr) {
        return sCtx.error("不支持的激活函数sigmod ");
    }

    m_pTanhActivator = CActivator::getActivation(idType, "tanh");
    if(m_pTanhActivator == nullptr) {
        return sCtx.error("不支持的激活函数tanh");
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


template<typename Q> int CGruNetwork::initWeightT(int nWeights, int nCells) {
    Q* pWeightsZ = new Q[nWeights];
    Q* pWeightsR = new Q[nWeights];
    Q* pWeightsH = new Q[nWeights];
    Q* pState = new Q[nCells];
    Q* pBais = new Q[nCells];
    m_spWeightsZ.take((char*)pWeightsZ, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spWeightsR.take((char*)pWeightsR, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spWeightsH.take((char*)pWeightsH, [](char* pWeights){
        delete[] (Q*)pWeights;
    });
    m_spState.take((char*)pState, [](char* pState){
        delete[] (Q*)pState;
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
        pWeightsZ[i] = -xWeight + CUtils::rand() * xWeight * 2;
        pWeightsR[i] = -xWeight + CUtils::rand() * xWeight * 2;
        pWeightsH[i] = -xWeight + CUtils::rand() * xWeight * 2;
    }
    for(int i=0; i<nCells; i++ ){
        pState[i] = 0;
    }

    return 0;
}

template<typename Q> int CGruNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut)
{
    int nBatchs = m_nBatchs;
    int nGroups = m_nGroups;
    int nTensors = nBatchs * nGroups;
    int nCells = m_nCells;
    int nOutputSize = m_nCells*sizeof(Q);
    int nInputCells = m_nInputCells;
    int nJoinCells = nCells + nInputCells;
    int nWeights = nInputCells*(nInputCells*nCells);
    bool bKeepGroup = m_bKeepGroup;

    //
    //执行内部状态值的指针，用于在最后一步，将状态计算结果(网络输出)保存下来
    //
    Q* pState = (Q*)(void*)m_spState;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
        Q* pWeightZ;
        Q* pWeightR;
        Q* pWeightH;
        Q* pState;
    }varBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spInternalBatchOut->getDataPtr<Q>(),
        (Q*)(void*)m_spWeightsZ,
        (Q*)(void*)m_spWeightsR,
        (Q*)(void*)m_spWeightsH,
        pState,
    };
    varBackup = it;

    IVectorSolver* pSolver = m_spSolver.getPtr();
    Q joinInVec[nJoinCells], hhInVec[nJoinCells], hhOutVec[nCells], zVec[nCells], rVec[nCells];
    for(int iTensor=0; iTensor<nTensors; iTensor++) {
        //
        // 输入向量和状态向量连接
        //
        if( pSolver->join({nCells, it.pState}, {nInputCells, it.pIn}, {nJoinCells, joinInVec}) != sCtx.success() ) {
            return sCtx.error("连接输入和状态向量异常");
        }

        //计算向量z
        if( pSolver->multiply({nJoinCells, joinInVec}, {nWeights, it.pWeightZ}, {nCells, zVec}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pSigmodActivator->activate(nCells, zVec, zVec);

        //计算向量r
        if( pSolver->multiply({nJoinCells, joinInVec}, {nWeights, it.pWeightR}, {nCells, rVec}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pSigmodActivator->activate(nCells, rVec, rVec);

        //计算向量hh0
        if( pSolver->multiply(nCells, it.pState, rVec, hhInVec) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        if( pSolver->join({nCells, hhInVec}, {nInputCells, it.pIn}, {nJoinCells, hhInVec}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }

        //计算向量hh
        if( pSolver->multiply({nJoinCells, hhInVec}, {nWeights, it.pWeightH}, {nCells, hhOutVec}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pTanhActivator->activate(nCells, hhOutVec, hhOutVec);

        //计算输出
        if( pSolver->addByWeight(nCells, hhOutVec, it.pState, zVec, it.pOut) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }

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

int CGruNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
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

template<typename Q> int CGruNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
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
    int nJoinCells = nCells + nInputCells;
    bool bKeepGroup = m_bKeepGroup;

    int nWeights = nCells * (nInputCells + nCells);
    int nVariables = nWeights * 3;
    Q* pWeightZDerivationArray = (Q*)m_spOptimizer->getDeviationPtr(nVariables);
    Q* pWeightRDerivationArray = pWeightZDerivationArray+nWeights;
    Q* pWeightHDerivationArray = pWeightRDerivationArray+nWeights;
    memset(pWeightZDerivationArray, 0 ,sizeof(Q)*(nVariables));

    Q pPrevOutDeviationArray[nCells];
    Q pJoinInStateDevia[nJoinCells], pJoinInState[nJoinCells];
    Q* pJoinIn = pJoinInState + nCells;
    Q* pJoinInDevia = pJoinInStateDevia + nCells;

    Q* pInDeviation = spBatchInDeviation->getDataPtr<Q>();
    memset(pInDeviation, 0, sizeof(Q)*nTensor*nInputCells);
    Q hhInVec[nJoinCells], hhInDeviaVec[nJoinCells], tmp[nCells];

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
        Q* pWeightZ;
        Q* pWeightZDeviation;
        Q* pWeightR;
        Q* pWeightRDeviation;
        Q* pWeightH;
        Q* pWeightHDeviation;
        Q* pState;
        Q* pHH;
        Q* pZ;
        Q* pR;
        Q* pHHDeiva;
        Q* pZDeiva;
        Q* pRDeiva;
        Q* pInState;
        Q* pInStateDevia;
    }it;

    it.pIn = spBatchIn->getDataPtr<Q>() + (nTensor-1)*nInputCells;
    it.pInDeviation = pInDeviation + (nTensor-1)*nInputCells;
    it.pOut = spBatchOut->getDataPtr<Q>() + (nTensor-1)*nCells;
    it.pOutDeviation = spBatchOutDeviation->getDataPtr<Q>() + (nTensor-1)*nCells;//这个地方小心
    it.pWeightZ = (Q*)(void*)m_spWeightsZ;
    it.pWeightZDeviation = pWeightZDerivationArray;
    it.pWeightR = (Q*)(void*)m_spWeightsR;
    it.pWeightRDeviation = pWeightRDerivationArray;
    it.pWeightH = (Q*)(void*)m_spWeightsH;
    it.pWeightHDeviation = pWeightHDerivationArray;
    it.pState = (Q*)(void*)m_spState;
    it.pHH = tmp;
    it.pZ = it.pHH + nCells;
    it.pR = it.pZ + nCells;
    it.pHHDeiva = it.pR + nCells;
    it.pRDeiva = it.pHHDeiva + nCells;
    it.pZDeiva = it.pRDeiva + nCells;
    it.pInState = pJoinInState;
    it.pInStateDevia = pJoinInStateDevia;

    IVectorSolver* pSolver = m_spSolver.getPtr();

    int iTensor, iGroupOut;
    int nGroupOut = m_bKeepGroup ? 1 : nGroups;
    for(iTensor=0, iGroupOut=0; iTensor<nTensor; iTensor++) {
        //
        //  非一组中最后一个输出时（非倒序的第一个），其偏差等于上一此计算的状态输入偏差，其它情况为实际输出偏差（由外层传入）
        //
        if(iGroupOut>0) {
            pSolver->copy(nCells, pPrevOutDeviationArray, pJoinInStateDevia);
            it.pOutDeviation = pPrevOutDeviationArray;
        }

        //
        // 初始化组合后的输入向量，以及输入偏导向量
        //
        pSolver->copy(nInputCells, pJoinIn, it.pIn);
        if(iTensor == 0) {
            pSolver->copy(nCells, pJoinInState, it.pState);
        }
        if(iTensor == nTensor-1) {
            pSolver->copy(nCells, pJoinInState, it.pOut-nCells);
        }else{
            //
            // 如果是第一个输入，由于之前的值没有存储，所以，将值设置为零，相当于已经无法学习到偏导数
            //
            pSolver->zero(nCells, pJoinInState);
        }

        //计算向量z
        if( pSolver->multiply({nJoinCells, pJoinIn}, {nWeights, it.pWeightZ}, {nCells, it.pZ}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pSigmodActivator->activate(nCells, it.pZ, it.pZ);

        //计算向量r
        if( pSolver->multiply({nJoinCells, pJoinIn}, {nWeights, it.pWeightR}, {nCells, it.pR}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pSigmodActivator->activate(nCells, it.pR, it.pR);

        //计算向量hh0
        if( pSolver->multiply(nCells, it.pState, it.pR, hhInVec) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        if( pSolver->join({nCells, hhInVec}, {nInputCells, it.pIn}, {nJoinCells, hhInVec}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }

        //计算向量hh
        if( pSolver->multiply({nJoinCells, hhInVec}, {nWeights, it.pWeightH}, {nCells, it.pHH}) != sCtx.success() ) {
            return sCtx.error("权重和输入向量相乘异常");
        }
        m_pTanhActivator->activate(nCells, it.pHH, it.pHH);

   
        //y = (1-z) * h + z * hh公式偏导
        {
            //求hh偏导: dy * z
            pSolver->multiply(nCells, it.pOutDeviation, it.pZ, it.pHHDeiva);
            m_pTanhActivator->deactivate(nCells, it.pHH, it.pHHDeiva, it.pHHDeiva);

            //求h偏导:dy * (1 - z)
            pSolver->multiply(nCells, it.pOutDeviation, it.pZ, it.pInStateDevia);
            pSolver->del(nCells, it.pOut, it.pInStateDevia, it.pInStateDevia);

            //求z偏导：dy * (hh-h)
            pSolver->del(nCells, it.pHH, it.pInState, it.pZDeiva);
            pSolver->multiply(nCells, it.pZDeiva, it.pOut, it.pZDeiva);
        }

        //hh=(W*(r*h,x))部分求导
        {
            pSolver->zero(nJoinCells, hhInDeviaVec);
            if( pSolver->multiplyAccDeviation(
                {nCells, it.pHH, it.pHHDeiva },
                {nWeights, it.pWeightH, it.pWeightHDeviation },
                {nJoinCells, hhInVec, hhInDeviaVec } ) != sCtx.success() ) {
                return sCtx.error("偏导数计算错误");
            }

            //r偏导数
            pSolver->multiply(nCells, hhInDeviaVec, it.pHHDeiva, it.pRDeiva);

            //x偏导数
            pSolver->add(nInputCells, it.pInDeviation, hhInDeviaVec+nCells);

            //求h(t-1)偏导数
            pSolver->multiply(nCells, hhInDeviaVec, it.pHHDeiva, hhInDeviaVec);
            pSolver->add(nCells, it.pInStateDevia, hhInDeviaVec);
        }

        //最后一部分
        {
            pSolver->zero(nJoinCells, pJoinInStateDevia);
            m_pSigmodActivator->deactivate(nCells, it.pR, it.pRDeiva, it.pRDeiva);
            m_pSigmodActivator->deactivate(nCells, it.pZ, it.pZDeiva, it.pZDeiva);
            if( pSolver->multiplyAccDeviation(
                {nCells, it.pR, it.pRDeiva },
                {nWeights, it.pWeightR, it.pWeightRDeviation },
                {nJoinCells, pJoinInState, pJoinInStateDevia } ) != sCtx.success() ) {
                return sCtx.error("偏导数计算错误");
            }
            if( pSolver->multiplyAccDeviation(
                {nCells, it.pZ, it.pZDeiva },
                {nWeights, it.pWeightZ, it.pWeightZDeviation },
                {nJoinCells, pJoinInState, pJoinInStateDevia } ) != sCtx.success() ) {
                return sCtx.error("偏导数计算错误");
            }
            pSolver->add(nInputCells, it.pInDeviation, pJoinInDevia);
        }

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
    pSolver->del(nWeights, it.pWeightZ, pWeightZDerivationArray);
    pSolver->del(nWeights, it.pWeightR, pWeightRDerivationArray);
    pSolver->del(nWeights, it.pWeightH, pWeightHDerivationArray);
    return sCtx.success();
}

int CGruNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {

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

int CGruNetwork::toArchive(const SIoArchive& ar) {
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
        ar.visitTaker("weightsZ", nBytes*m_nCells*(m_nInputCells+m_nCells), m_spWeightsZ);
        ar.visitTaker("weightsR", nBytes*m_nCells*(m_nInputCells+m_nCells), m_spWeightsR);
        ar.visitTaker("weightsH", nBytes*m_nCells*(m_nInputCells+m_nCells), m_spWeightsH);
        ar.visitTaker("state", nBytes*m_nCells, m_spState);
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGruNetwork, CGruNetwork::__getClassKey())