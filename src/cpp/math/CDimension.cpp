#include "CDimension.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CDimension");

int CDimension::getSize() {
    return m_nElementSize;
}

const int* CDimension::getData() {
    return (const int*)m_spData;
}

int CDimension::getElementSize() {
    const int* pDimSizes = getData();
    int nElementSize = 1;
    for(int i=0; i<m_nElementSize; i++) {
        nElementSize *= pDimSizes[i];
    }
    return nElementSize;
}

int CDimension::toArchive(const SArchive& ar) {
    ar.arBlock("size", m_nElementSize);
    ar.visitTaker("data", m_nElementSize, m_spData);
    return sCtx.success();
}

int CDimension::__initialize(const PData* pData) {
    const PVector* pV = CData<PVector>(pData);
    if( pV == nullptr ) {
        return sCtx.error("参数错误");
    }

    int size = pV->size;
    m_nElementSize = pV->size;
    if(size > 0) {
        if(size == 1) {
            if(*pV->pIntArray == 1) {
                m_nElementSize = 0;
                return sCtx.success();
            }
        }
        if(pV->pIntArray == nullptr) {
            return sCtx.error("参数错误");
        }
        m_spData.take(new int[size], [](int* pPtr){
            delete[] pPtr;
        });
        memcpy(m_spData, pV->pIntArray, size*sizeof(int));
    }
    return sCtx.success();
}

int CDimension::createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
    if(nDims > 0 && pDimSizes == nullptr) {
        return sCtx.error("参数错误");
    }

    if(nDims == 0 || ( nDims == 1 && pDimSizes[0] == 1)) {
        static PVector sZeroVector = {0};
        static SDimension sZeroDimension = SObject::createObject<SDimension>(CData<PVector>(sZeroVector));
        spDim = sZeroDimension;
        return sCtx.success(); 
    }

    CPointer<CDimension> spTensor;
    CObject::createObject(spTensor);
    spTensor->m_nElementSize = nDims;
    spTensor->m_spData.take(new int[nDims], [](int* pPtr){
        delete[] pPtr;
    });
    memcpy(spTensor->m_spData, pDimSizes, nDims*sizeof(int));
    spDim.setPtr(spTensor.getPtr());
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDimension, CDimension::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE
