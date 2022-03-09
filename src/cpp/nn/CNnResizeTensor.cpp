
#include "CNnResizeTensor.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnResizeTensor");

int CNnResizeTensor::__initialize(const PData* pData){
    const PNnResizeTensor* pInitializer = CData<PNnResizeTensor>(pData);
    if(pInitializer == nullptr) {
        return sCtx.error("缺少初始化参数");
    }
    m_src = *pInitializer;
    return sCtx.success();
}

int CNnResizeTensor::createResizeTensor(const PNnResizeTensor& rTenser, STensor& spTensor) {
    CPointer<CNnResizeTensor> spPointer;
    CObject::createObject(spPointer);
    spPointer->m_src = rTenser;
    spTensor.setPtr(spPointer.getPtr());
    return sCtx.success();
}

int CNnResizeTensor::getDimension(SDimension& spDim) {
    spDim = m_src.spTensor.dimension();
    return sCtx.success();
}

PDATATYPE CNnResizeTensor::getDataType(){
    return m_src.spTensor.type();
}

int CNnResizeTensor::getDataSize() {
    return m_src.spTensor.size();
}

int CNnResizeTensor::toDevice(const SDevice& spDevice, PVector* pDeviceData) {
    return m_src.spTensor->toDevice(spDevice, pDeviceData);
}

void* CNnResizeTensor::getDataPtr(PDATATYPE eElementType, int iPos) {
    return m_src.spTensor->getDataPtr(eElementType, iPos);
}

int CNnResizeTensor::toArchive(const SArchive& ar) {
    ar.arObject("src", m_src.spTensor);
    ar.arObject("extra1", m_src.spExtra1);
    ar.arObject("extra2", m_src.spExtra2);
    return sCtx.success();
}

int CNnResizeTensor::getResizeData(PNnResizeTensor& rResizeTensor) {
    rResizeTensor = m_src;
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnResizeTensor, CNnResizeTensor::__getClassKey())