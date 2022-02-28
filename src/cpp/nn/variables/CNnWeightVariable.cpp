
#include "variable.h"

static SCtx sCtx("CNnWeightVariable");
class CNnWeightVariable : public CObject, public INnVariable, public INnInternalVariable, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(INnInternalVariable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Weight"; }

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVWeight; }
    void* getData(unsigned int idType);
    int getDimension(SDimension& spDimension) {
        spDimension = m_spDimension;
        return sCtx.success();
    }
    int getSize() {
        return m_spDimension.dataSize();
    }

private:
    template<typename Q> void initWeightT(int nWeights, void* pWeights);

private:
    SDimension m_spDimension;
    STensor m_spData;
    float m_dAvg;
};

int CNnWeightVariable::__initialize(const PData* pData) {
    const PNnWeight* pWeight = CData<PNnWeight>(pData);
    if(pWeight == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_spDimension = pWeight->spDim;
    m_dAvg = pWeight->dAvg;
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
    Q xWeight = (Q)0.1;
    for(int i=0; i<nWeights; i++) {
        ((Q*)pWeights)[i] = (Q)m_dAvg + -xWeight + CUtils::rand() * xWeight * 2;
    }
}

int CNnWeightVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    ar.arObject("value", m_spData);
    ar.arBlock("avg", m_dAvg);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnWeightVariable, CNnWeightVariable::__getClassKey())