#include "CNnWeightVariable.h"

static SCtx sCtx("CNnWeightVariable");

int CNnWeightVariable::__initialize(const PData* pData) {
    const PNnWeight* pWeight = CData<PNnWeight>(pData);
    if(pWeight == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_spDimension = pWeight->spDim;
    m_dAvg = pWeight->dAvg;
    return sCtx.success();
}

int CNnWeightVariable::createWeightVariable(const SDimension& spDimension, SNnVariable& spOutVar) { 
    CPointer<CNnWeightVariable> spWeight;
    CObject::createObject(spWeight);
    spWeight->m_spDimension = spDimension;
    spOutVar.setPtr(spWeight.getPtr());
    return sCtx.success();
}

void* CNnWeightVariable::getData(unsigned int idType) {
    if(!m_spData) {
        int nDims = m_spDimension.size();
        const int* pDimSize = m_spDimension.data();

        int nData = 1;
        for(int i=0; i<nDims; i++) {
            nData *= pDimSize[i];
        }

        if(STensor::createTensor(m_spData, m_spDimension, idType, nData) != sCtx.success()) {
            return nullptr;
        }

        void* pData = m_spData->getDataPtr(idType);
        if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nData, pData);
        }else
        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nData, pData);
        }
    }
    return m_spData->getDataPtr(idType);
}

template<typename Q> void CNnWeightVariable::initWeightT(int nWeights, void* pWeights) {
    Q xWeight = 0.1;
    for(int i=0; i<nWeights; i++) {
        ((Q*)pWeights)[i] = m_dAvg + -xWeight + CUtils::rand() * xWeight * 2;
    }
}

int CNnWeightVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    ar.arObject("value", m_spData);
    ar.arBlock("avg", m_dAvg);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnWeightVariable, CNnWeightVariable::__getClassKey())