
#include "CActivatorUnit.h"

static SCtx sCtx("CActivatorUnit");

template<typename Q> int CActivatorUnit::initT(const STensor& spInDim, STensor& spOutDim) {
    return sCtx.success();
}

template<typename Q>
int CActivatorUnit::evalT(const Q* pIn, Q* pOut){
    m_pActivator->activate(m_nCells, (void*)pIn, pOut);
    return sCtx.success();
}

template<typename Q>
int CActivatorUnit::learnT(const Q* pIn, const Q* pOut, const Q* pOutDev, Q* pInDev) {
    m_pActivator->deactivate(m_nCells, (void*)pOut, (void*)pOutDev, pInDev);
    return sCtx.success();
}

template<typename Q>
int CActivatorUnit::updateWeightsT() {
    return sCtx.success();
}

int CActivatorUnit::createUnit(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szActivator) {
    CPointer<CActivatorUnit> sp;
    CObject::createObject(sp);
    sp->m_nCells = nCells;
    sp->m_pActivator = CActivator::getActivation(idType, szActivator);
    sp->m_idType = idType;
    if(szActivator != nullptr)
        sp->m_strActivator = szActivator;
    if(sp->m_pActivator == nullptr) {
        return sCtx.error("初始化神经网络单元失败");
    }
    spUnit.setPtr(sp.getPtr());
    return sCtx.success();
}

int CActivatorUnit::toArchive(const SIoArchive& ar) {
    //基础参数
    ar.visit("cells", m_nCells);
    ar.visitString("activator", m_strActivator);
    ar.visit("dataType", m_idType);
    if(ar->isReading()) {
        m_pActivator = CActivator::getActivation(m_idType, m_strActivator.c_str());
        if(m_pActivator == nullptr) {
            return sCtx.error("初始化神经网络单元失败");
        }
    }
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CActivatorUnit, CActivatorUnit::__getClassKey())