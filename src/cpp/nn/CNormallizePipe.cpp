#include "CNormallizePipe.h"

static SCtx sCtx("CNormalizePipe");
int CNormalizePipe::push(const STensor& spIn, STensor& spOut) {

    int nSize = spIn->getDataSize();
    if( STensor::createTensor<double>(spOut, spIn->getDimVector(), nSize ) != sCtx.success() ) {
        return sCtx.error("创建标准化张量失败");
    }

    unsigned int iInType = spIn->getDataType();
    if(iInType == CBasicData<unsigned char>::getStaticType() ) {
        unsigned char* pSrc = spIn->getDataPtr<unsigned char>();
        double* pDesc = spOut->getDataPtr<double>();
        for(int i=0; i<nSize; i++) {
            pDesc[i] = pSrc[i] / 255.0;
        }
        return sCtx.success();
    }

    return sCtx.error("不支持的归一化输入类型");
}

int CNormalizePipe::createPipe(SNeuralPipe& spPipe) {
    CPointer<CNormalizePipe> spReader;
    CObject::createObject(spReader);
    spPipe.setPtr(spReader.getPtr());
    return sCtx.success();
}