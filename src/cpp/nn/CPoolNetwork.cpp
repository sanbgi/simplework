#include "CPoolNetwork.h"
#include "iostream"

SCtx CPoolNetwork::sCtx("CPoolNetwork");
int CPoolNetwork::createNetwork(int nWidth, int nHeight, int nStrideWidth, int nStrideHeight, SNnNetwork& spNetwork) {
    CPointer<CPoolNetwork> spPool;
    CObject::createObject(spPool);
    spPool->m_nPoolWidth = nWidth;
    spPool->m_nPoolHeight = nHeight;
    spPool->m_nStrideWidth = nStrideWidth;
    spPool->m_nStrideHeight = nStrideHeight;
    spNetwork.setPtr(spPool.getPtr());
    return sCtx.success();
}

int CPoolNetwork::prepareNetwork(const STensor& spBatchIn) {

    //
    // 快速检查数量（非严格检查）, 如果严格对比长宽高的化，有点浪费性能，相当于如果
    // 两次输入张量尺寸相同，则细节维度尺寸就按照上次维度尺寸进行
    //
    int nInputSize = spBatchIn->getDataSize();
    if( nInputSize == m_nBatchInSize ) {
        return sCtx.success();
    }

    //
    // 计算参数
    //
    STensor& spInDimVector = spBatchIn->getDimVector();
    int nDims = spInDimVector->getDataSize();
    int* pDimSizes = spInDimVector->getDataPtr<int>();
    if(nDims < 3) {
        return sCtx.error("池化层输入张量维度需要大于等于3，其中第一个维度为实际张量个数");
    }

    int nBatchs = pDimSizes[0];
    int nInputHeight = pDimSizes[1];
    int nInputWidth = pDimSizes[2];
    if( nInputHeight < m_nPoolHeight || nInputWidth < m_nPoolWidth ) {
        return sCtx.error("输入张量尺寸需要大于等于卷积核尺寸");
    }

    int nLayers = 1;
    for( int i=3; i<nDims; i++ ) {
        nLayers *= pDimSizes[i];
    }

    int nInputCells = nInputWidth*nInputHeight*nLayers;
    if(nBatchs * nInputCells != nInputSize ) {
        return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
    }

    m_nBatchs = pDimSizes[0];
    m_nInputHeight = pDimSizes[1];
    m_nInputWidth = pDimSizes[2];
    m_nInputLayer = 1;
    int pOutDimSizes[nDims];
    for( int i=3; i<nDims; i++) {
        pOutDimSizes[i] = pDimSizes[i];
        m_nInputLayer *= pDimSizes[i];
    }
    m_nOutHeight = (m_nInputHeight - m_nPoolHeight) / m_nStrideHeight + 1;
    m_nOutWidth = (m_nInputWidth - m_nPoolWidth) / m_nStrideWidth + 1;
    m_nOutTensorSize = m_nOutHeight * m_nOutWidth * m_nInputLayer;
    m_nInputTensorSize = m_nInputHeight * m_nInputWidth * m_nInputLayer;
    pOutDimSizes[0] = m_nBatchs;
    pOutDimSizes[1] = m_nOutHeight;
    pOutDimSizes[2] = m_nOutWidth;
    if( STensor::createVector(m_spOutDimVector, nDims, pOutDimSizes) != sCtx.success() ) {
        return sCtx.error("创建输出张量的维度张量失败");
    }

    if( STensor::createTensor<double>(m_spBatchOut, m_spOutDimVector, m_nOutTensorSize * m_nBatchs) != sCtx.success() ) {
        return sCtx.error("创建输出张量失败");
    }
    
    m_nBatchInSize = nInputCells * nBatchs;
    return sCtx.success();
}

int CPoolNetwork::eval(const STensor& spInTensor, STensor& spOutTensor) {
    if(prepareNetwork(spInTensor) != sCtx.success()) {
        return sCtx.error();
    }

    int nPoolWidth = m_nPoolWidth;
    int nPoolHeight = m_nPoolHeight;
    int nOutWidth = m_nOutWidth;
    int nOutHeight = m_nOutHeight;
    int nTensor = m_nBatchs;
    int nLayer = m_nInputLayer;
    int nInputTensorSize = m_nInputTensorSize;
    int nOutputTensorSize = m_nOutTensorSize;

    int nInputHstep = m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * m_nStrideHeight;
    int nInStrideWstep = nInputWstep * m_nStrideWidth;

    int nOutHstep = m_nOutWidth * nLayer;
    int nOutWstep = nLayer;

    struct CItOutVariables {
        double* pIn;
        double* pOut;
    }it = {
        spInTensor->getDataPtr<double>(),
        m_spBatchOut->getDataPtr<double>(),
    };
    for(int iTensor=0; iTensor<nTensor; iTensor++) {
        CItOutVariables varTBackup = {
            it.pIn,
            it.pOut,
        };
        for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
            CItOutVariables varOYBackup;
            varOYBackup.pIn = it.pIn;
            varOYBackup.pOut = it.pOut;
            for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                CItOutVariables varOXBackup;
                varOXBackup.pIn = it.pIn;
                varOXBackup.pOut = it.pOut;
                for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                    CItOutVariables varConvBackup;
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pOut = it.pOut;

                    double dMax = (*it.pIn);

                    //
                    //  从输入中找到最大的那个值
                    //  
                    for( int iConvY=0; iConvY<nPoolHeight; iConvY++) {
                        CItOutVariables varConvYBackup;
                        varConvYBackup.pIn = it.pIn;
                        for( int iConvX=0; iConvX<nPoolWidth; iConvX++) {
                            if( (*it.pIn) > dMax) {
                                dMax = (*it.pIn);
                            }
                            it.pIn += nInputWstep;
                        }
                        it.pIn = varConvYBackup.pIn + nInputHstep;
                    }

                    (*it.pOut) = dMax;

                    it.pIn = varConvBackup.pIn + 1;
                    it.pOut = varConvBackup.pOut + 1;;
                }
                it.pIn = varOXBackup.pIn + nInStrideWstep;
                it.pOut = varOXBackup.pOut + nOutWstep;
            }
            it.pIn = varOYBackup.pIn + nInStrideHstep;
            it.pOut = varOYBackup.pOut + nOutHstep;
        }
        it.pIn = varTBackup.pIn + nInputTensorSize;
        it.pOut = varTBackup.pOut + nOutputTensorSize;
    }
    
    m_spBatchIn = spInTensor;
    spOutTensor = m_spBatchOut;
    return sCtx.success();
}

int CPoolNetwork::learn(const STensor& spOutTensor, const STensor& spOutDeviation, STensor& spInTensor, STensor& spInDeviation) {
    if(spOutTensor.getPtr() != m_spBatchOut.getPtr()) {
        return sCtx.error("神经网络已经更新，原有数据不能用于学习");
    }

    spInTensor = m_spBatchIn;
    if( int errCode = STensor::createTensor<double>(spInDeviation, spInTensor->getDimVector(), spInTensor->getDataSize()) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输入偏差张量失败");
    }

    double* pDeviationInputArray = spInDeviation->getDataPtr<double>();
    memset(pDeviationInputArray,0,sizeof(double)*spInTensor->getDataSize());

    int nLayer = m_nInputLayer;
    int nInputWidth = m_nInputWidth;
    int nInputHeight = m_nInputHeight;
    int nInputLayer = m_nInputLayer;
    int nPoolWidth = m_nPoolWidth;
    int nPoolHeight = m_nPoolHeight;
    int nOutWidth = m_nOutWidth;
    int nOutHeight = m_nOutHeight;

    int nTensor = m_nBatchs;
    int nInputTensorSize = m_nInputTensorSize;
    int nOutputTensorSize = m_nOutTensorSize;

    int nInputHstep = m_nInputWidth * nLayer;
    int nInputWstep = nLayer;

    int nInStrideHstep = nInputHstep * m_nStrideHeight;
    int nInStrideWstep = nInputWstep * m_nStrideWidth;
    
    int nOutHstep = nOutWidth * nLayer;
    int nOutWstep = nLayer;

    struct CItOutVariables {
        double* pIn;
        double* pInDeviation;
        double* pOutDeviation;
    }it = {
        spInTensor->getDataPtr<double>(),
        spInDeviation->getDataPtr<double>(),
        spOutDeviation->getDataPtr<double>(),
    };
    for(int iTensor=0; iTensor<nTensor; iTensor++) {
        CItOutVariables varTBackup = {
            it.pIn,
            it.pInDeviation,
            it.pOutDeviation
        };
        for( int iOutY=0; iOutY < nOutHeight; iOutY++) {
            CItOutVariables varOYBackup;
            varOYBackup.pIn = it.pIn;
            varOYBackup.pInDeviation = it.pInDeviation;
            varOYBackup.pOutDeviation = it.pOutDeviation;
            for( int iOutX=0; iOutX < nOutWidth; iOutX++) {
                CItOutVariables varOXBackup;
                varOXBackup.pIn = it.pIn;
                varOXBackup.pInDeviation = it.pInDeviation;
                varOXBackup.pOutDeviation = it.pOutDeviation;
                for( int iLayer = 0; iLayer < nLayer; iLayer++) {
                    CItOutVariables varConvBackup;
                    varConvBackup.pIn = it.pIn;
                    varConvBackup.pInDeviation = it.pInDeviation;
                    varConvBackup.pOutDeviation = it.pOutDeviation;

                    double dMax = (*it.pIn);
                    double* pExpectDelta = it.pInDeviation;

                    //
                    //  从输入中找到最大的那个点，作为反向传到的点
                    //  
                    for( int iConvY=0; iConvY<nPoolHeight; iConvY++) {
                        CItOutVariables varConvYBackup;
                        varConvYBackup.pIn = it.pIn;
                        varConvYBackup.pInDeviation = it.pInDeviation;
                        for( int iConvX=0; iConvX<nPoolWidth; iConvX++) {
                            if( (*it.pIn) > dMax) {
                                dMax = (*it.pIn);
                                pExpectDelta = it.pInDeviation;
                            }
                            it.pIn += nInputWstep;
                            it.pInDeviation += nInputWstep;
                        }
                        it.pIn = varConvYBackup.pIn + nInputHstep;
                        it.pInDeviation = varConvYBackup.pInDeviation + nInputHstep;
                    }

                    (*pExpectDelta) = (*it.pOutDeviation);

                    it.pIn = varConvBackup.pIn + 1;
                    it.pInDeviation = varConvBackup.pInDeviation + 1;
                    it.pOutDeviation = varConvBackup.pOutDeviation + 1;;
                }

                it.pIn = varOXBackup.pIn + nInStrideWstep;
                it.pInDeviation = varOXBackup.pInDeviation + nInStrideWstep;
                it.pOutDeviation = varOXBackup.pOutDeviation + nOutWstep;
            }

            it.pIn = varOYBackup.pIn + nInStrideHstep;
            it.pInDeviation = varOYBackup.pInDeviation + nInStrideHstep;
            it.pOutDeviation = varOYBackup.pOutDeviation + nOutHstep;
        }

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nInputTensorSize;
        it.pInDeviation = varTBackup.pInDeviation + nInputTensorSize;
        it.pOutDeviation = varTBackup.pOutDeviation + nOutputTensorSize;
    }
    return sCtx.success();
}

int CPoolNetwork::toArchive(const SIoArchive& ar) {
    ar.visit("nPoolWidth", m_nPoolWidth);
    ar.visit("nPoolHeight", m_nPoolHeight);
    ar.visit("nStrideWidth", m_nStrideWidth);
    ar.visit("nStrideHeight", m_nStrideHeight);
    ar.visitString("padding", m_strPadding);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CPoolNetwork, CPoolNetwork::__getClassKey())