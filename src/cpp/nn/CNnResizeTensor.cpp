
#include "CNnResizeTensor.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnResizeTensor");

int CNnResizeTensor::__initialize(const PData* pData){
    const PNnResizeTensor* pInitializer = CData<PNnResizeTensor>(pData);
    if(pInitializer == nullptr) {
        return sCtx.error("缺少初始化参数");
    }
    m_spTensor = pInitializer->spTensor;
    for(int i=0; i<pInitializer->nExtras; i++) {
        m_arrExtras.push_back(pInitializer->pExtras[i]);
    }
    return sCtx.success();
}

int CNnResizeTensor::createResizeTensor(const PNnResizeTensor& rTenser, STensor& spTensor) {
    CPointer<CNnResizeTensor> spPointer;
    CObject::createObject(spPointer);
    if(spPointer->__initialize(CData<PNnResizeTensor>(rTenser)) != sCtx.success()) {
        return sCtx.error("初始化错误");
    }
    spTensor.setPtr(spPointer.getPtr());
    return sCtx.success();
}

int CNnResizeTensor::getDimension(SDimension& spDim) {
    spDim = m_spTensor.dimension();
    return sCtx.success();
}

PDATATYPE CNnResizeTensor::getDataType(){
    return m_spTensor.type();
}

int CNnResizeTensor::getDataSize() {
    return m_spTensor.size();
}

int CNnResizeTensor::getDataInDevice(const SDevice& spDevice, PVector& deviceData) {
    return m_spTensor->getDataInDevice(spDevice, deviceData);
}

int CNnResizeTensor::toArchive(const SArchive& ar) {
    ar.arObject("src", m_spTensor);
    ar.arObjectArray("extras", m_arrExtras);
    return sCtx.success();
}

int CNnResizeTensor::getResizeData(PNnResizeTensor& rResizeTensor) {
    rResizeTensor.spTensor = m_spTensor;
    rResizeTensor.nExtras = m_arrExtras.size();
    rResizeTensor.pExtras = m_arrExtras.data();
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnResizeTensor, CNnResizeTensor::__getClassKey())