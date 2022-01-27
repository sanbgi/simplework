#include "CNnWeightVariable.h"
#include "CUtils.h"
static SCtx sCtx("CNnWeightVariable");
int CNnWeightVariable::createWeightVariable(const SDimension& spDimension, SNnVariable& spOutVar) { 
    CPointer<CNnWeightVariable> spWeight;
    CObject::createObject(spWeight);
    spWeight->m_spDimension = spDimension;
    spOutVar.setPtr(spWeight.getPtr());
    return 0;
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
    Q xWeight = 0.1;//sqrt(1.0/(m_nConvWidth*m_nConvHeight*nInLayers));
    for(int i=0; i<nWeights; i++) {
        //pWeights[i] = 0;
        ((Q*)pWeights)[i] = -xWeight + CUtils::rand() * xWeight * 2;
    }
}

int CNnWeightVariable::toArchive(const SIoArchive& ar) {
    ar.visitObject("dimension", m_spDimension);
    ar.visitObject("value", m_spData);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnWeightVariable, CNnWeightVariable::__getClassKey())