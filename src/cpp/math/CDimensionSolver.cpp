#include "../inc/math/math.h"
#include <map>
#include <vector>
#include "CTensor.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

static SCtx sCtx("CDimensionSolver");

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CDimensionSolver : public CObject, IDimensionSolver {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IDimensionSolver)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
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

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CDimensionSolver, SDimensionSolver::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE