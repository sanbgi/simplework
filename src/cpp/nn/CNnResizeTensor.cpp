
#include "CNnResizeTensor.h"

using namespace sw;
using namespace std;

static SCtx sCtx("CNnResizeTensor");

int CNnResizeTensor::__initialize(const PData* pData){
    const PNnResizeTensor* pInitializer = CData<PNnResizeTensor>(pData);
    if(pInitializer == nullptr) {
        return sCtx.error("缺少初始化参数");
    }

    if(pInitializer->iResizeOffset <= 0 || pInitializer->iResizeOffset >= pInitializer->spSrc.size() ) {
        return sCtx.error("不允许偏移位置超出原始张量尺寸范围");
    }

    if(pInitializer->spResizeDimension.dataSize() + pInitializer->iResizeOffset < pInitializer->spSrc.size() ) {
        return sCtx.error("不允许调整尺寸后的张量大于原始张量");
    }

    m_src = *pInitializer;
    return sCtx.success();
}

int CNnResizeTensor::createResizeTensor(const PNnResizeTensor& rTenser, STensor& spTensor) {
    if(rTenser.iResizeOffset < 0 || rTenser.iResizeOffset >= rTenser.spSrc.size() ) {
        return sCtx.error("不允许偏移位置超出原始张量尺寸范围");
    }

    int sizeSrc = rTenser.spSrc.size();
    int sizeResize = rTenser.spResizeDimension.dataSize();
    if( sizeResize + rTenser.iResizeOffset > sizeSrc ) {
        return sCtx.error("不允许调整尺寸后的张量大于原始张量");
    }

    CPointer<CNnResizeTensor> spPointer;
    CObject::createObject(spPointer);
    spPointer->m_src = rTenser;
    spTensor.setPtr(spPointer.getPtr());
    return sCtx.success();
}

int CNnResizeTensor::getDimension(SDimension& spDim) {
    spDim = m_src.spResizeDimension;
    return sCtx.success();
}

PID CNnResizeTensor::getDataType(){
    return m_src.spSrc.type();
}

int CNnResizeTensor::getDataSize() {
    return m_src.spResizeDimension.dataSize();
}

void* CNnResizeTensor::getDataPtr(PID eElementType, int iPos) {
    return m_src.spSrc->getDataPtr(eElementType, iPos+m_src.iResizeOffset);
}

int CNnResizeTensor::toArchive(const SArchive& ar) {
    ar.arObject("src", m_src.spSrc);
    ar.arObject("dimension", m_src.spResizeDimension);
    ar.arBlock("ioffset", m_src.iResizeOffset);
    ar.arObject("extra", m_src.spExtra);
    return sCtx.success();
}

int CNnResizeTensor::getResizeData(PNnResizeTensor& rResizeTensor) {
    rResizeTensor = m_src;
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnResizeTensor, CNnResizeTensor::__getClassKey())