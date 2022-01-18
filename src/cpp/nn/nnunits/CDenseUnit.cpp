
#include "CDenseUnit.h"

static SCtx sCtx("CDenseUnit");

template<typename Q> int CDenseUnit::initT(int nCells, const STensor& spInDim) {
    int nDims = spInDim.size();
    int* pDims = spInDim.data<int>();
    if( nDims < 1 || pDims == nullptr ) {
        return sCtx.error("输入维度信息无效");
    }

    int nInputCells = 1;
    for(int i=0; i<nDims; i++) {
        nInputCells *= pDims[i];
    }
    
    int nWeights = nInputCells*nCells;
    m_idType = CBasicData<Q>::getStaticType();
    Q* pWeights = new Q[nWeights];
    m_spWeights.take((char*)pWeights, [](char* pWeights){
        delete[] (Q*)pWeights;
    });

    Q xWeight = 0.1;//sqrt(1.0/nInputCells);
    for(int i=0; i<nWeights; i++) {
        pWeights[i] = -xWeight + CUtils::rand() * xWeight * 2;
    }
    m_nCells = nCells;
    m_nInputCells = nInputCells;
    return sCtx.success();
}

template<typename Q>
int CDenseUnit::evalT(const Q* pIn, Q* pOut){
    int nCells = m_nCells;
    int nInputCells = m_nInputCells;
    int iIn, iOut;
    const Q* pInIt;
    Q dOut;
    Q* pWeights = (Q*)(char*)m_spWeights;
    for(iOut=0; iOut<nCells; iOut++) {
        dOut = 0;
        pInIt = pIn;
        for(iIn=0; iIn<nInputCells; iIn++) {
            dOut += (*pInIt) * (*pWeights);
            pInIt++, pWeights++;
        }
        (*pOut) = dOut;
        pOut++;
    }
    return sCtx.success();
}

template<typename Q>
int CDenseUnit::learnT(const Q* pIn, const Q* pOut, const Q* pOutDev, Q* pInDev) {
    int iIn, iOut;
    double dOutDev;
    Q* pInDevIt;
    const Q* pInIt;
    int nCells = m_nCells;
    int nInputCells = m_nInputCells;
    int nWeights = nCells*nInputCells;
    Q* pWeightDeviationArray = (Q*)m_spOptimizer->getDeviationPtr(nWeights);
    if(m_nBatchs == 0){
        memset(pWeightDeviationArray, 0 ,sizeof(Q)*(nWeights));
    }

    Q* pWeights = (Q*)(char*)m_spWeights;
    Q* pWeightDevs = pWeightDeviationArray;
    for(iOut=0; iOut<nCells; iOut++) {
        dOutDev = *pOutDev;
        pInIt = pIn;
        pInDevIt = pInDev;
        for(iIn=0; iIn<nInputCells; iIn++) {
            *pInDevIt += dOutDev * (*pWeights);
            (*pWeightDevs) += dOutDev * (*pInIt);
            pInIt++, pInDevIt++, pWeights++, pWeightDevs++;
        }
        pOut++, pOutDev++;
    }
    m_nBatchs++;
    return sCtx.success();
}

template<typename Q>
int CDenseUnit::updateWeightsT() {
    if(m_nBatchs > 0) {
        m_spOptimizer->updateDeviation(m_nBatchs);
        int nWeights = m_nCells*m_nInputCells;
        Q* pWeightDevs = (Q*)m_spOptimizer->getDeviationPtr(nWeights);
        Q* pWeightArray = (Q*)(char*)m_spWeights;
        Q* pWeightEnd = pWeightArray+nWeights;
        while(pWeightArray != pWeightEnd) {
            *pWeightArray -= *pWeightDevs;
            pWeightArray++, pWeightDevs++;
        }
    }
    m_nBatchs = 0;
    return sCtx.success();
}

int CDenseUnit::createUnit(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer, const STensor& spInDim) {
    CPointer<CDenseUnit> sp;
    CObject::createObject(sp);
    if( COptimizer::getOptimizer(szOptimizer, idType, sp->m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    if(idType==CBasicData<double>::getStaticType()) {
        if( sp->initT<double>(nCells, spInDim) != sCtx.success() ) {
            return sCtx.error("初始化神经网络单元失败");
        }
    }else
    if(idType==CBasicData<float>::getStaticType()){
        if( sp->initT<float>(nCells, spInDim) != sCtx.success() ) {
            return sCtx.error("初始化神经网络单元失败");
        }
    }else{
        return sCtx.error("数据类型不支持，初始化神经网络单元失败");
    }
    spUnit.setPtr(sp.getPtr());
    return sCtx.success();
}

int CDenseUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visit("inputCells", m_nInputCells);
    ar.visitString("optimizer", m_strOptimizer);
    ar.visit("dataType", m_idType);
    int nBytes = CType::getTypeBytes(m_idType);
    int nWeight = m_nCells * m_nInputCells;
    ar.visitTaker("weights", nBytes*nWeight, m_spWeights);
    if(ar->isReading()) {
        if( COptimizer::getOptimizer(m_strOptimizer.c_str(), m_idType, m_spOptimizer) != sCtx.success()) {
            return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
        }
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDenseUnit, CDenseUnit::__getClassKey())