#include "../inc/math/math.h"
#include <map>
#include <vector>
#include "CTensor.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CTensorSolver");

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CTensorSolver : public CObject, ITensorSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ITensorSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int minus( const STensor& t1, const STensor& t2, STensor& spOut) {
        int nSize = t1->getDataSize();
        if( nSize != t2->getDataSize()) {
            return sCtx.error("大小不同的两个张量不能相减");
        }

        int idType1 = t1.type();
        int idType2 = t2.type();
        if(idType1 != idType2 ) {
            return sCtx.error("类型不同的两个张量无法相减");
        }

        STensor spRet;
        if( STensor::createTensor(spRet, t1.dimension(), idType1, nSize) != sCtx.success() ) {
            return sCtx.error("创建结果张量失败");
        }

        void* pT1 = t1->getDataPtr(idType1);
        void* pT2 = t2->getDataPtr(idType1);
        void* pOut = spRet->getDataPtr(idType1);
        if(idType1 == CBasicData<float>::getStaticType()) {
            for(int i=0; i<nSize; i++) {
                ((float*)pOut)[i] = ((float*)pT1)[i] - ((float*)pT2)[i];
            }
        }else if(idType1 == CBasicData<double>::getStaticType()){
            for(int i=0; i<nSize; i++) {
                ((double*)pOut)[i] = ((double*)pT1)[i] - ((double*)pT2)[i];
            }
        }else{
            return sCtx.error("不支持的数据类型");
        }
        spOut = spRet;
        return sCtx.success();
    }

    //
    // 升维
    //
    int upHighDimension(const SDimension& spIn, int nDims, int pDimSizes[], SDimension& spOut) {
        int nPrevDims = spIn.size();
        const int* pPrevDimSizes = spIn.data();

        std::vector<int> pNewDimSizes(nPrevDims+nDims);
        for(int i=0; i<nPrevDims; i++) {
            pNewDimSizes[nDims+i] = pPrevDimSizes[i];
        }
        for(int i=0; i<nDims; i++) {
            pNewDimSizes[i] = pDimSizes[i];
        }
        spOut = SDimension(nPrevDims+nDims,pNewDimSizes.data());
        return sCtx.success();
    }

    //
    // 降维
    //
    int downHighDimension(const SDimension& spIn, int nDims, SDimension& spOut){
        int nPrevDims = spIn.size();
        if(nPrevDims-nDims < 1) {
            return sCtx.error("一维一下维度，无法降维");
        }
        const int* pDimSizes = spIn.data();
        spOut = SDimension(nPrevDims-nDims,pDimSizes+1);
        return sCtx.success();
    }

    //
    // 升维
    //
    int upLowDimension(const SDimension& spIn, int nDims, int pDimSizes[], SDimension& spOut) {
        int nPrevDims = spIn.size();
        const int* pPrevDimSizes = spIn.data();

        std::vector<int> pNewDimSizes(nPrevDims+nDims);
        for(int i=0; i<nPrevDims; i++) {
            pNewDimSizes[i] = pPrevDimSizes[i];
        }
        for(int i=0; i<nDims; i++) {
            pNewDimSizes[nPrevDims+i] = pDimSizes[i];
        }
        spOut = SDimension(nPrevDims+nDims,pNewDimSizes.data());
        return sCtx.success();
    }

    //
    // 降维
    //
    int downLowDimension(const SDimension& spIn, int nDims, SDimension& spOut){
        int nPrevDims = spIn.size();
        if(nPrevDims - nDims < 1) {
            sCtx.error("一维一下维度，无法降维");
        }
        const int* pDimSizes = spIn.data();
        spOut = SDimension(nPrevDims-nDims,pDimSizes);
        return sCtx.success();
    }

    bool isEqual(const SDimension& spDim1, const SDimension& spDim2) {
        if(spDim1.getPtr() == spDim2.getPtr()) {
            return true;
        }

        int nSize1 = spDim1.size();
        int nSize2 = spDim2.size();
        const int* pDimSize1 = spDim1.data();
        const int* pDimSize2 = spDim2.data();
        if( nSize1 != nSize2 ) {
            return false;
        }

        while(nSize1>0) {
            if(*pDimSize2 != *pDimSize1) {
                return false;
            }
            nSize1--, pDimSize1++, pDimSize2++;
        }
        return true;
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CTensorSolver, STensorSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE