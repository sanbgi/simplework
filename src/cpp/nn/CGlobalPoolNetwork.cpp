#include "CGlobalPoolNetwork.h"
#include "CType.h"
#include <math.h>
#include <iostream>
#include <time.h>

static SCtx sCtx("CGlobalPoolNetwork");
int CGlobalPoolNetwork::createNetwork(const char* szMode, const char* szActivator, SNnNetwork& spNetwork) {
    CPointer<CGlobalPoolNetwork> spPool;
    CObject::createObject(spPool);
    if( szActivator!=nullptr )
        spPool->m_strActivator = szActivator;
    if( szMode != nullptr ) {
        spPool->m_strMode = szMode;
    }
    spNetwork.setPtr(spPool.getPtr());
    return sCtx.success();
}

int CGlobalPoolNetwork::prepareNetwork(const STensor& spBatchIn) {
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
    int nLayers = 0;
    int nLayerSize = 0;
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
        nLayers = pDimSizes[nInputDims-1];
        nLayerSize = 1;
        for( int i=1; i<nInputDims-1; i++) {
            nLayerSize *= pDimSizes[i];
        }
        if(nBatchs * nLayerSize * nLayers != nInputSize ) {
            return sCtx.error("输入张量的维度信息核实际数据量不一致，输入张量非法");
        }
    }

    //
    // 检查细胞数量是否合法
    //
    int pOutDimSizes[2] = { nBatchs, nLayers };
    STensor spOutDimVector;
    if( int errCode = STensor::createVector(spOutDimVector, 2, pOutDimSizes) != sCtx.success() ) {
        return sCtx.error(errCode, "创建神经网络输出张量维度向量失败");
    }

    if( int errCode = STensor::createTensor(m_spBatchOut, spOutDimVector, idType, nBatchs * nLayers) != sCtx.success() ) {
        return sCtx.error(errCode, "创建输出张量失败");
    }

    m_pActivator = CActivator::getActivation(idType, m_strActivator.c_str());
    if(m_pActivator == nullptr) {
        return sCtx.error((std::string("不支持的激活函数名: ") + m_strActivator).c_str());
    }

    m_spBatchInDeviation.release();
    m_nBatchs = nBatchs;
    m_nLayers = nLayers;
    m_nLayerSize = nLayerSize;
    m_idDataType = idType;
    return sCtx.success();
}

template<typename Q> int CGlobalPoolNetwork::evalT(const STensor& spBatchIn, STensor& spBatchOut)
{
    int nBatchs = m_nBatchs;
    int nLayers = m_nLayers;
    int nLayerSize = m_nLayerSize;
    int nTensorSize = m_nLayerSize*nLayers;
    struct CItOutVariables {
        Q* pIn;
        Q* pOut;
    }varTBackup, varOBackup, it = {
        spBatchIn->getDataPtr<Q>(),
        m_spBatchOut->getDataPtr<Q>(),
    };

    Q dOut;
    int iTensor, iOutput, iInput;
    for(iTensor=0; iTensor<nBatchs; iTensor++) {
        varTBackup.pIn = it.pIn;
        varTBackup.pOut = it.pOut;
        for(iOutput=0; iOutput<nLayers; iOutput++) {
            varOBackup.pIn = it.pIn;
            varOBackup.pOut = it.pOut;

            dOut = 0;
            for(iInput=0; iInput<nLayerSize; iInput++ ) {
                dOut += (*it.pIn);
                it.pIn++;
            }

            (*it.pOut) = dOut / nLayerSize;

            //  更新迭代参数
            it.pOut++;
            it.pIn = varOBackup.pIn + nLayerSize;
        }
        
        m_pActivator->activate(nLayers, varTBackup.pOut, varTBackup.pOut);

        //  更新迭代参数
        it.pIn = varTBackup.pIn + nTensorSize;
        it.pOut = varTBackup.pOut + nLayers;
    }

    m_spBatchIn = spBatchIn;
    spBatchOut = m_spBatchOut;
    return sCtx.success();
}

int CGlobalPoolNetwork::eval(const STensor& spBatchIn, STensor& spBatchOut) {
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

template<typename Q> int CGlobalPoolNetwork::learnT(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
    if( !m_spBatchInDeviation ) {
        if( int errCode = STensor::createTensor(m_spBatchInDeviation, m_spBatchIn->getDimVector(), m_idDataType, m_spBatchIn->getDataSize()) != sCtx.success() ) {
            return sCtx.error(errCode, "创建输入偏差张量失败");
        }
    }
    spBatchIn = m_spBatchIn;
    spBatchInDeviation = m_spBatchInDeviation;

    int nBatchs = m_nBatchs;
    int nLayers = m_nLayers;
    int nLayerSize = m_nLayerSize;
    int nTensorSize = m_nLayerSize*nLayers;
    struct CItOutVariables {
        Q* pInDeviation;
        Q* pOut;
        Q* pOutDeviation;
        Q* pZDeviatioin;
    }varTBackup, varOBackup, it = {
        spBatchInDeviation->getDataPtr<Q>(),
        spBatchOut->getDataPtr<Q>(),
        spBatchOutDeviation->getDataPtr<Q>(),
    };
    memset(it.pInDeviation, 0, sizeof(Q)*m_nInputSize);

    Q pZDerivationArray[nLayers], deviationZ;
    int iTensor, iOutput, iInput;
    for(iTensor=0; iTensor<nBatchs; iTensor++) {
        varTBackup.pInDeviation = it.pInDeviation;
        varTBackup.pOut = it.pOut;
        varTBackup.pOutDeviation = it.pOutDeviation;

        //
        //  计算目标函数相对于Y值的偏导数
        //
        m_pActivator->deactivate(m_nLayers, it.pOut, it.pOutDeviation, pZDerivationArray);
        it.pZDeviatioin = pZDerivationArray;

        //
        //  调整权重
        //
        for(iOutput=0; iOutput<nLayers; iOutput++) {
            varOBackup.pInDeviation = it.pInDeviation;

            deviationZ = *(it.pZDeviatioin);
            if(deviationZ > 1.0e-16 || deviationZ < -1.0e-16) {
                deviationZ /= nTensorSize;
                for(iInput=0; iInput<nTensorSize; iInput++ ) {
                    (*it.pInDeviation) += deviationZ;
                    it.pInDeviation++;
                }
            }

            //  更新迭代参数
            it.pZDeviatioin++;
            it.pInDeviation = varOBackup.pInDeviation + nLayerSize;
        }

        //  更新迭代参数
        it.pInDeviation = varTBackup.pInDeviation + nTensorSize;
        it.pOut = varTBackup.pOut + nLayers;
        it.pOutDeviation = varTBackup.pOutDeviation + nLayers;
    }
    return sCtx.success();
}

int CGlobalPoolNetwork::learn(const STensor& spBatchOut, const STensor& spBatchOutDeviation, STensor& spBatchIn, STensor& spBatchInDeviation) {
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

int CGlobalPoolNetwork::toArchive(const SIoArchive& ar) {
    ar.visitString("mode", m_strMode);
    ar.visitString("activator", m_strActivator);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CGlobalPoolNetwork, CGlobalPoolNetwork::__getClassKey())