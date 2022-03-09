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
    if(pDimSizes == nullptr) {
        return sCtx.error("当前向量并非维度向量");
    }
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

int CDimension::createDimension(SDimension& spDim, int nDims, const int* pDimSizes) {
        CPointer<CDimension> spTensor;
        CObject::createObject(spTensor);
        spTensor->m_nElementSize = nDims;
        spTensor->m_spData.take(new int[nDims], [](int* pPtr){
            delete[] pPtr;
        });
        if(pDimSizes) {
            memcpy(spTensor->m_spData, pDimSizes, nDims*sizeof(int));
        }
        spDim.setPtr(spTensor.getPtr());
        return sCtx.success();
    }

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDimension, CDimension::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE
