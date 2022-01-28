#include "CNnStateVariable.h"

static SCtx sCtx("CNnStateVariable");
int CNnStateVariable::createState(const SDimension& spDimension, SNnState& spOutVar) { 
    CPointer<CNnStateVariable> spState;
    CObject::createObject(spState);
    spState->m_spDimension = spDimension;
    spOutVar.setPtr(spState.getPtr());
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
    ar.visitObject("dimension", m_spDimension);
    ar.visitObject("value", m_spData);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnStateVariable, CNnStateVariable::__getClassKey())