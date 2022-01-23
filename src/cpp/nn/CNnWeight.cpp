#include "CNnWeight.h"
static SCtx sCtx("CNnWeight");
int CNnWeight::createWeightVariable(const STensor& spDimVector, SNnVariable& spOutVar) { 
    CPointer<CNnWeight> spWeight;
    CObject::createObject(spWeight);
    spWeight->m_spDimVector = spDimVector;
    spOutVar.setPtr(spWeight.getPtr());
    return 0;
}

void* CNnWeight::getData(unsigned int idType) {
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