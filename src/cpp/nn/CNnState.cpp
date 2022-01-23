#include "CNnState.h"

static SCtx sCtx("CNnState");
int CNnState::createStateVariable(const SDimension& spDimension, SNnVariable& spOutVar) { 
    CPointer<CNnState> spState;
    CObject::createObject(spState);
    spState->m_spDimension = spDimension;
    spOutVar.setPtr(spState.getPtr());
    return sCtx.success();
}

int CNnState::createStateVariable(int nDims, const int pDimSizes[], SNnVariable& spOutVar) {
    SDimension spDim;
    if( SDimension::createDimension(spDim, nDims, pDimSizes) != sCtx.success() ) {
        return sCtx.error("创建维度失败");
    }
    return createStateVariable(spDim, spOutVar);
}

void* CNnState::getData(unsigned int idType) {
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