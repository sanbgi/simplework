
#include "CBaisUnit.h"

static SCtx sCtx("CBaisUnit");

template<typename Q> int CBaisUnit::initT(int nCells) {
    m_idType = CBasicData<Q>::getStaticType();
    Q* pBais = new Q[nCells];
    m_spBais.take((char*)pBais, [](char* pBais){
        delete[] (Q*)pBais;
    });
    for(int i=0; i<nCells; i++ ){
        pBais[i] = 0;
    }
    m_nCells = nCells;
    return sCtx.success();
}

template<typename Q>
int CBaisUnit::evalT(const Q* pIn, Q* pOut){
    int nCells = m_nCells;
    Q* pBais = (Q*)(void*)m_spBais;
    for(int i=0; i<nCells; i++) {
        *pOut = *pIn - *pBais;
        pIn++, pOut++, pBais++; 
    }
    return sCtx.success();
}

template<typename Q>
int CBaisUnit::learnT(const Q* pIn, const Q* pOut, const Q* pOutDev, Q* pInDev) {
    int nCells = m_nCells;
    Q* pBais = (Q*)(void*)m_spBais;
    Q* pBaisDeviationArray = (Q*)m_spOptimizer->getDeviationPtr(nCells);
    const Q* pOutDeviation = pOutDev;
    if(m_nBatchs == 0){
        memset(pBaisDeviationArray, 0 ,sizeof(Q)*(nCells));
    }

    for(int i=0; i<nCells; i++) {
        *pBaisDeviationArray += -*pOutDeviation;
        pBaisDeviationArray++, pOutDeviation++; 
    }

    if(pInDev !=pOutDev) {
        for(int i=0; i<nCells; i++) {
            *pInDev = *pOutDev;
            pInDev++, pOutDev++; 
        }
    }
    m_nBatchs++;
    return sCtx.success();
}

template<typename Q>
int CBaisUnit::updateWeightsT() {
    if(m_nBatchs > 0) {
        m_spOptimizer->updateDeviation(m_nBatchs);

        int nWeights = m_nCells;
        Q* pWeightDevs = (Q*)m_spOptimizer->getDeviationPtr(nWeights);
        Q* pWeightArray = (Q*)(char*)m_spBais;
        Q* pWeightEnd = pWeightArray+nWeights;
        while(pWeightArray != pWeightEnd) {
            *pWeightArray -= *pWeightDevs;
            pWeightArray++, pWeightDevs++;
        }
    }
    m_nBatchs = 0;
    return sCtx.success();
}

int CBaisUnit::createUnit(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer) {
    CPointer<CBaisUnit> sp;
    CObject::createObject(sp);
    sp->m_nCells = nCells;
    if( COptimizer::getOptimizer(szOptimizer, idType, sp->m_spOptimizer) != sCtx.success()) {
        return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
    }

    if(idType==CBasicData<double>::getStaticType()) {
        if( sp->initT<double>(nCells) != sCtx.success() ) {
            return sCtx.error("初始化神经网络单元失败");
        }
    }else
    if(idType==CBasicData<float>::getStaticType()){
        if( sp->initT<float>(nCells) != sCtx.success() ) {
            return sCtx.error("初始化神经网络单元失败");
        }
    }else{
        return sCtx.error("数据类型不支持，初始化神经网络单元失败");
    }
    spUnit.setPtr(sp.getPtr());
    return sCtx.success();
}


int CBaisUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visitString("optimizer", m_strOptimizer);
    ar.visit("dataType", m_idType);
    int nBytes = CType::getTypeBytes(m_idType);
    ar.visitTaker("bais", nBytes*m_nCells, m_spBais);
    if(ar->isReading()) {
        if( COptimizer::getOptimizer(m_strOptimizer.c_str(), m_idType, m_spOptimizer) != sCtx.success()) {
            return sCtx.error((std::string("创建梯度下降优化器失败 ")).c_str());
        }
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CBaisUnit, CBaisUnit::__getClassKey())