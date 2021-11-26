#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include "../../core/inc/Core.h"
#include "ITensor.h"

namespace SimpleWork {

/**
 * 张量类定义
 */
SIMPLEWORK_CLASS_ENTER(Tensor)
    //
    // 构造张量
    //
    template<typename T> Tensor(int nData, T* pData, int nDim = 1, int* pDimSizeArr = nullptr) {
        getFactory()->createObject(this);
        getPtr()->initTensor(ITensor::getDataType<T>(), nDim, pDimSizeArr, nData, pData);
    };

    //
    // 获取数据大小
    //
    int getDataSize() {
        return getPtr()->getDataSize();
    }

    //
    // 获取数据指针
    //
    template<typename T> T* getDataPtr(int iPos=0) {
        ITensor* pTensor = getPtr();
        if( ITensor::getDataType<T>() == pTensor->getDataType() ) {
            pTensor->getDataPtr(iPos);
        }
        return nullptr;
    }

    //
    // 获取维度大小
    //
    int getDimSize() {
        return getPtr()->getDimSize();
    }

    //
    // 获取维度指针
    // 
    int* getDimPtr(int iPos=0) {
        return getPtr()->getDimPtr(iPos);
    }

    //
    // 获取数据
    //
    template<typename T> T& getData(int iPos) {
        return nullptr;
    }
    template<typename T> T& getData(int* pPosArr) {
        return nullptr;
    }

    //
    // 修改数据
    //
    template<typename T> int setData(int iPos, T& v) {
        return 0;
    }
    template<typename T> int setData(int* pPosArr, T& v ) {
        return 0;
    }

private:
    static IFactory* getFactory() {
        static SmartPtr<IFactory> s_ptrFactory = getSimpleWorkModule()->createFactory("SimpleWork.Tensor");
        return s_ptrFactory.getPtr();
    };

SIMPLEWORK_CLASS_LEAVE

}//namespace SimpleWork

#endif//__SimpleWork_Tensor_h__