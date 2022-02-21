#include "variable.h"

static SCtx sCtx("CNnStateVariable");

class CNnStateVariable : public CNnVariable, public INnState, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(INnState)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnVariable)

public://CObject
    int __initialize(const PData* pData);

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "State"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://INnState
    int getDimension(SDimension& spDimension) {
        return CNnVariable::getDimension(spDimension);
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.State"; }
    
private:
    ENnVariableType getVariableType() { return ENnVariableType::EVState; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
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

void* CNnStateVariable::getData(unsigned int idType) {
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
    return m_spData->getDataPtr(idType);
}


int CNnStateVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    ar.arObject("value", m_spData);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnStateVariable, CNnStateVariable::__getClassKey())