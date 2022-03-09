#include "variable.h"

static SCtx sCtx("CNnStateVariable");

class CNnStateVariable : public CObject, public INnVariable, public INnState, public INnInternalVariable, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(INnState)
        SIMPLEWORK_INTERFACE_ENTRY(INnInternalVariable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.State"; }
    
private:
    ENnVariableType getVariableType() { return ENnVariableType::EVState; }
    void* getData(PDATATYPE idType);

    int getDimension(SDimension& spDimension) {
        spDimension = m_spDimension;
        return 0;
    }
    int getSize() {
        return m_spDimension.dataSize();
    }

private:
    SDimension m_spDimension;
    STensor m_spData;
};

int CNnStateVariable::__initialize(const PData* pData) {
    const PNnState* pState = CData<PNnState>(pData);
    if(pState == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_spDimension = pState->spDim;
    return sCtx.success();
}

void* CNnStateVariable::getData(PDATATYPE idType) {
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
    }
    return m_spData.data();
}

int CNnStateVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    ar.arObject("value", m_spData);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnStateVariable, CNnStateVariable::__getClassKey())