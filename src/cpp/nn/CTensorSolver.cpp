#include "CTensorSolver.h"

static SCtx sCtx("CTensorSolver");
int CTensorSolver::normalize(const STensor& spIn, STensor& spOut) {

    int nSize = spIn->getDataSize();
    STensor spOutTensor;
    if( STensor::createTensor<double>(spOutTensor, spIn.dimension(), nSize ) != sCtx.success() ) {
        return sCtx.error("创建标准化张量失败");
    }

    unsigned int iInType = spIn->getDataType();
    if(iInType == CBasicData<unsigned char>::getStaticType() ) {
        unsigned char* pSrc = spIn->getDataPtr<unsigned char>();
        double* pDesc = spOutTensor->getDataPtr<double>();
        for(int i=0; i<nSize; i++) {
            pDesc[i] = pSrc[i] / 255.0;
        }
        spOut = spOutTensor;
        return sCtx.success();
    }

    return sCtx.error("不支持的归一化输入类型");
}

int CTensorSolver::classify(int nClassify, const STensor& spIn, STensor& spOut) {

    //
    // 计算新张量尺寸
    //
    SDimension spDimVector = spIn.dimension();
    int nDims = spDimVector.size();
    const int* pDimSizes = spDimVector.data();
    int pNewDimSizes[nDims+1];
    for(int i=0; i<nDims; i++) {
        pNewDimSizes[i] = pDimSizes[i];
    }
    pNewDimSizes[nDims] = nClassify;
    SDimension spNewDimVector;
    SDimension::createDimension(spNewDimVector, nDims+1, pNewDimSizes);

    //
    // 创建新的张量
    //
    int nSize = spIn->getDataSize();
    STensor spOutTensor;
    if( STensor::createTensor<double>(spOutTensor, spNewDimVector, nSize*nClassify ) != sCtx.success() ) {
        return sCtx.error("创建标准化张量失败");
    }

    //
    //  计算分类化向量值
    //
    unsigned int iInType = spIn->getDataType();
    if(iInType == CBasicData<unsigned char>::getStaticType() ) {
        unsigned char* pSrc = spIn->getDataPtr<unsigned char>();
        double* pDesc = spOutTensor->getDataPtr<double>();
        for(int i=0; i<nSize; i++) {
            for( int j=0; j<nClassify; j++) {
                pDesc[i*nClassify+j] = (pSrc[i] == j) ? 1.0 : 0.0;
            }
        }
        spOut = spOutTensor;
        return sCtx.success();
    }

    return sCtx.error("不支持的归一化输入类型");
}