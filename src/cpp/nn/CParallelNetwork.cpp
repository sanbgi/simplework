#include "CParallelNetwork.h"

static SCtx sCtx("CParallelNetwork");
int CParallelNetwork::createNetwork(int nNetworks, SNnNetwork* pNetworks, SNnNetwork& spNetwork) {
    CPointer<CParallelNetwork> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nNetworks; i++) {
        spSequence->m_arrNetworks.push_back( *(pNetworks+i) );
    }
    spNetwork.setPtr(spSequence.getPtr());
    return sCtx.success();
}

template<typename Q>
int CParallelNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut) {

    STensor spInDimTensor = spBatchIn->getDimVector();
    int nInDims = spInDimTensor->getDataSize();
    if( nInDims < 2) {
        return sCtx.error("输入张量维度不能小于2");
    }

    int nSumLayers = 0;
    int nSumLayerSize = 0;
    STensor spSumDimTensor;
    std::vector<STensor>& arrOuts = m_arrOutTensors;
    arrOuts.clear();

    //
    // 所有节点求值，并把结果保存在m_arrOutTensors中
    //
    std::vector<SNnNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor spOut;
        if( int errCode = (*it)->eval(spBatchIn, spOut) != sCtx.success() ){
            return errCode;
        }
        STensor spDimTensor = spOut->getDimVector();
        int nDimSize = spDimTensor->getDataSize();
        if( nDimSize < 2) {
            return sCtx.error("子网络计算输出的张量维度少于2");
        }

        int* pDimSize = spDimTensor->getDataPtr<int>();
        int nData = spOut->getDataSize();
        int nLayer = pDimSize[nDimSize-1];
        int nLayerSize = nData/nLayer;

        if(nSumLayers == 0) {
            nSumLayerSize = nLayerSize;
            spSumDimTensor = spDimTensor;
        }else if(nSumLayerSize != nLayerSize){
            return sCtx.error("并行网络的输出元素尺寸不一致");
        }
        nSumLayers += nLayer;
        arrOuts.push_back(spOut);
        it++;
    }

    //
    // 创建合并的输出张量
    //
    if(!m_spBatchOut) {
        int nDims = spSumDimTensor->getDataSize();
        int* pDimSize = spSumDimTensor->getDataPtr<int>();
        int pNewDimSize[nDims];
        for( int i=0; i<nDims-1; i++) {
            pNewDimSize[i] = pDimSize[i];
        }
        pNewDimSize[nDims-1] = nSumLayers;
        STensor spDimTensor = STensor::createVector(nDims,pNewDimSize);
        if( !spDimTensor ) {
            return sCtx.error("创建输出张量的维度尺寸张量失败");
        }

        if( STensor::createTensor<Q>(m_spBatchOut, spDimTensor, nSumLayers * nSumLayerSize) != sCtx.success()) {
            return sCtx.error("创建输出张量失败");
        }
        m_nOutLayers = nSumLayers;
        m_nOutLayerSize = nSumLayerSize;
    }else{
        if( m_spBatchOut->getDataSize() != nSumLayers * nSumLayerSize ) {
            return sCtx.error("两次计算的输出张量尺寸不一致，计算错误");
        }
    }

    //
    // 拷贝所有节点输出数据到统一的输出张量中
    //
    Q* pAllOutData = m_spBatchOut->getDataPtr<Q>();
    std::vector<STensor>::iterator itTensor = arrOuts.begin();
    while(itTensor != arrOuts.end()) {
        STensor spOut = *itTensor;
        int nData = spOut->getDataSize();
        Q* pData = spOut->getDataPtr<Q>();
        int nLayer = nData / nSumLayerSize;
        Q* pOut = pAllOutData;
        Q* pLayerAllData;
        Q* pLayerData;
        int iTensor, iLayer;
        for(iTensor=0; iTensor<nSumLayerSize; iTensor++) {
            pLayerAllData = pOut;
            pLayerData = pData;
            for(iLayer=0; iLayer<nLayer; iLayer++) {
                *pLayerAllData = *pLayerData;
                pLayerAllData++;
                pLayerData++;
            }
            pOut += nSumLayers;
            pData += nLayer;
        }
        pAllOutData += nLayer;
        itTensor++;
    }

    spBatchOut = m_spBatchOut;
    m_spBatchIn = spBatchIn;
    m_spBatchOut.updateVer();
    m_nInVer = m_spBatchIn.ver();
    m_nOutVer = m_spBatchOut.ver();
    return sCtx.success();
}

template<typename Q>
int CParallelNetwork::learnT(const STensor& spBatchOut, const STensor& spOutDeviation, STensor& spBatchIn, STensor& spInDeviation) {
    if(!m_spInDeviation) {
        int nIn = m_spBatchIn->getDataSize();
        if( STensor::createTensor<Q>(m_spInDeviation, m_spBatchIn->getDimVector(), nIn) != sCtx.success()) {
            return sCtx.error("创建输入偏差张量失败");
        }
    }else{
        m_spInDeviation.updateVer();
    }

    Q* pOutDeviation = spOutDeviation->getDataPtr<Q>();
    int nInDeviation = m_spInDeviation->getDataSize();
    Q* pInAllDeviation = m_spInDeviation->getDataPtr<Q>();
    memset(pInAllDeviation, 0, nInDeviation*sizeof(Q));
    Q* pBackupInDeviation = pInAllDeviation;

    int iOutLayer = 0;
    int nOutAllLayer = m_nOutLayers;
    int nOutLayerSize = m_nOutLayerSize;
    std::vector<STensor>::iterator itTensor = m_arrOutTensors.begin();
    std::vector<SNnNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor& spOut = *itTensor;
        STensor& spOutDim = spOut->getDimVector();

        int nOutDim = spOutDim->getDataSize();
        int nOutLayer = *spOutDim->getDataPtr<int>(nOutDim-1);

        STensor spOutDev;
        if( STensor::createTensor<Q>(spOutDev, spOut->getDimVector(), spOut->getDataSize()) != sCtx.success()) {
            return sCtx.error("创建节点输出偏差张量失败");
        }

        Q* pOutAllDev = pOutDeviation+iOutLayer;
        Q* pOutDev = spOutDev->getDataPtr<Q>();
        Q* pLayerAllDev;
        Q* pLayerDev;
        int iTensor, iLayer;
        for(iTensor=0; iTensor<nOutLayerSize; iTensor++) {
            pLayerAllDev = pOutAllDev;
            pLayerDev = pOutDev;
            for(iLayer=0; iLayer<nOutLayer; iLayer++) {
                *pLayerDev = *pLayerAllDev;
                pLayerAllDev++;
                pLayerDev++;
            }
            pOutDev += nOutLayer;
            pOutAllDev += nOutAllLayer;
        }
         
        STensor spIn;
        STensor spInDev;
        if( int errCode = (*it)->learn(spOut, spOutDev, spIn, spInDev) != sCtx.success() ) {
            return errCode;
        }

        if(spInDev->getDataSize() != nInDeviation) {
            return sCtx.error("子节点返回的输入偏导，与并行网络整个输入的偏导不一致");
        }

        Q* pInDev = spInDev->getDataPtr<Q>();
        pInAllDeviation = pBackupInDeviation;
        for(int i=0; i<nInDeviation; i++) {
            *pInAllDeviation += *pInDev;
            pInDev++;
            pInAllDeviation++;
        }
        it++;
        itTensor++;
        iOutLayer += nOutLayer;
    }
    spBatchIn = m_spBatchIn;
    spInDeviation = m_spInDeviation;
    return sCtx.success();
}



int CParallelNetwork::prepareNetwork(const STensor& spBatchIn) {
    unsigned int idType = spBatchIn.type();
    m_idDataType = idType;
    return sCtx.success();
}

int CParallelNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
    if( int errCode = prepareNetwork(spBatchIn) != sCtx.success() ) {
        return errCode;
    }
    
    if(spBatchIn.getPtr() == m_spBatchIn.getPtr() && spBatchIn.ver() == m_nInVer ) {
        spBatchOut = m_spBatchOut;
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

int CParallelNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if(spBatchOut.getPtr() != m_spBatchOut.getPtr() || spBatchOut.ver() != m_nOutVer) {
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

int CParallelNetwork::toArchive(const SIoArchive& ar) {
    ar.visitObjectArray("nodes", m_arrNetworks);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CParallelNetwork, CParallelNetwork::__getClassKey())