#include "CParallelNetwork.h"

SCtx CParallelNetwork::sCtx("CParallelNetwork");
int CParallelNetwork::createNetwork(int nNetworks, SNeuralNetwork* pNetworks, SNeuralNetwork& spNetwork) {
    CPointer<CParallelNetwork> spSequence;
    CObject::createObject(spSequence);
    for(int i=0; i<nNetworks; i++) {
        spSequence->m_arrNetworks.push_back( *(pNetworks+i) );
    }
    spNetwork.setPtr(spSequence.getPtr());
    return sCtx.success();
}

int CParallelNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {

    STensor spInDimTensor = spInTensor->getDimVector();
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
    std::vector<SNeuralNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor spOut;
        if( int errCode = (*it)->eval(spInTensor, spOut) != sCtx.success() ){
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
    if(!m_spOutTensor) {
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

        if( STensor::createTensor<double>(m_spOutTensor, spDimTensor, nSumLayers * nSumLayerSize) != sCtx.success()) {
            return sCtx.error("创建输出张量失败");
        }
        m_nOutLayers = nSumLayers;
        m_nOutLayerSize = nSumLayerSize;
    }else{
        if( m_spOutTensor->getDataSize() != nSumLayers * nSumLayerSize ) {
            return sCtx.error("两次计算的输出张量尺寸不一致，计算错误");
        }
    }

    //
    // 拷贝所有节点输出数据到统一的输出张量中
    //
    double* pAllOutData = m_spOutTensor->getDataPtr<double>();
    std::vector<STensor>::iterator itTensor = arrOuts.begin();
    while(itTensor != arrOuts.end()) {
        STensor spOut = *itTensor;
        int nData = spOut->getDataSize();
        double* pData = spOut->getDataPtr<double>();
        int nLayer = nData / nSumLayerSize;
        double* pOut = pAllOutData;
        double* pLayerAllData;
        double* pLayerData;
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

    spOutTensor = m_spOutTensor;
    m_spInTensor = spInTensor;
    return sCtx.success();
}

int CParallelNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spOutTensor.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    if(!m_spInDeviation) {
        int nIn = m_spInTensor->getDataSize();
        if( STensor::createTensor<double>(m_spInDeviation, m_spInTensor->getDimVector(), nIn) != sCtx.success()) {
            return sCtx.error("创建输入偏差张量失败");
        }
    }

    // 检查并准备输入输出偏导数指针
    if(spOutDeviation->getDataSize() != spOutTensor->getDataSize()) {
        return sCtx.error("学习的偏差张量尺寸，与最初输出的张量尺寸不一致");
    }
    double* pOutDeviation = spOutDeviation->getDataPtr<double>();
    int nInDeviation = m_spInDeviation->getDataSize();
    double* pInAllDeviation = m_spInDeviation->getDataPtr<double>();
    memset(pInAllDeviation, 0, nInDeviation*sizeof(double));
    double* pBackupInDeviation = pInAllDeviation;

    int iOutLayer = 0;
    int nOutAllLayer = m_nOutLayers;
    int nOutLayerSize = m_nOutLayerSize;
    std::vector<STensor>::iterator itTensor = m_arrOutTensors.begin();
    std::vector<SNeuralNetwork>::iterator it = m_arrNetworks.begin();
    while(it != m_arrNetworks.end() ) {
        STensor& spOut = *itTensor;
        STensor& spOutDim = spOut->getDimVector();

        int nOutDim = spOutDim->getDataSize();
        int nOutLayer = *spOutDim->getDataPtr<int>(nOutDim-1);

        STensor spOutDev;
        if( STensor::createTensor<double>(spOutDev, spOut->getDimVector(), spOut->getDataSize()) != sCtx.success()) {
            return sCtx.error("创建节点输出偏差张量失败");
        }

        double* pOutAllDev = pOutDeviation+iOutLayer;
        double* pOutDev = spOutDev->getDataPtr<double>();
        double* pLayerAllDev;
        double* pLayerDev;
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

        double* pInDev = spInDev->getDataPtr<double>();
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
    spInTensor = m_spInTensor;
    spInDeviation = m_spInDeviation;
    return sCtx.success();
}
