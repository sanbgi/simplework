
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
    STensor getData(PDATATYPE idType);
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

STensor CNnWeightVariable::getData(PDATATYPE idType) {
    if(!m_spData) {
        int nSize = m_spDimension.dataSize();
        if(STensor::createTensor(m_spData, m_spDimension, idType, nSize) != sCtx.success()) {
            return STensor();
        }

        void* pData = m_spData.data();
        if(idType == CBasicData<float>::getStaticType()) {
            initWeightT<float>(nSize, pData);
        }else
        if(idType == CBasicData<double>::getStaticType()) {
            initWeightT<double>(nSize, pData);
        }
    }
    return m_spData;
}

template<typename Q> void CNnWeightVariable::initWeightT(int nWeights, void* pWeights) {
    Q xWeight = (Q)0.1;
    for(int i=0; i<nWeights; i++) {
        //double r = i/777.1*10234665;
        //r = r - (int)r;
        //((Q*)pWeights)[i] = (Q)m_dAvg + -xWeight + r * xWeight * 2;
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