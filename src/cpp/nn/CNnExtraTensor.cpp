
#include "CNnExtraTensor.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnExtraTensor");

int CNnExtraTensor::__initialize(const PData* pData){
    const PNnExtraTensor* pInitializer = CData<PNnExtraTensor>(pData);
    if(pInitializer == nullptr) {
        return sCtx.error("缺少初始化参数");
    }
    m_spTensor = pInitializer->spTensor;
    for(int i=0; i<pInitializer->nExtras; i++) {
        m_arrExtras.push_back(pInitializer->pExtras[i]);
    }
    return sCtx.success();
}

int CNnExtraTensor::createResizeTensor(const PNnExtraTensor& rTenser, STensor& spTensor) {
    CPointer<CNnExtraTensor> spPointer;
    CObject::createObject(spPointer);
    if(spPointer->__initialize(CData<PNnExtraTensor>(rTenser)) != sCtx.success()) {
        return sCtx.error("初始化错误");
    }
    spTensor.setPtr(spPointer.getPtr());
    return sCtx.success();
}

int CNnExtraTensor::getDimension(SDimension& spDim) {
    return m_spTensor->getDimension(spDim);
}

int CNnExtraTensor::getDataBuffer(SDeviceMemory& spMemory){
    return m_spTensor->getDataBuffer(spMemory);
}

PDATATYPE CNnExtraTensor::getDataType(){
    return m_spTensor.type();
}

int CNnExtraTensor::getDataSize() {
    return m_spTensor.size();
}

int CNnExtraTensor::toArchive(const SArchive& ar) {
    ar.arObject("src", m_spTensor);
    ar.arObjectArray("extras", m_arrExtras);
    return sCtx.success();
}

int CNnExtraTensor::getResizeData(PNnExtraTensor& rResizeTensor) {
    rResizeTensor.spTensor = m_spTensor;
    rResizeTensor.nExtras = m_arrExtras.size();
    rResizeTensor.pExtras = m_arrExtras.data();
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnExtraTensor, CNnExtraTensor::__getClassKey())