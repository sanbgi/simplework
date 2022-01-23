#include "CDimension.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CDimension");

int CDimension::getSize() {
    return m_nElementSize;
}

const int* CDimension::getData() {
    return (const int*)getDataPtr(CBasicData<int>::getStaticType());
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

int CDimension::getVector(STensor& spDimVector) {
    spDimVector.setPtr((ITensor*)this);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CDimension, CDimension::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE
