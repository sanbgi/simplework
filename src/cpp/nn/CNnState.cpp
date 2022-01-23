#include "CNnState.h"

static SCtx sCtx("CNnState");
int CNnState::createStateVariable(const STensor& spDimVector, SNnVariable& spOutVar) { 
    CPointer<CNnState> spState;
    CObject::createObject(spState);
    spState->m_spDimVector = spDimVector;
    spOutVar.setPtr(spState.getPtr());
    return 0;
}


void* CNnState::getData(unsigned int idType) {
    if(!m_spData) {
        int nDims = m_spDimVector.size();
        const int* pDimSize = m_spDimVector.data();

        int nData = 1;
        for(int i=0; i<nDims; i++) {
            nData *= pDimSize[i];
        }

        if(STensor::createTensor(m_spData, m_spDimVector, idType, nData) != sCtx.success()) {
            return nullptr;
        }
    }
    return m_spData->getDataPtr(idType);
}