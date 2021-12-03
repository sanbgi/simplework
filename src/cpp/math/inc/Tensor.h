#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include <typeinfo>

using namespace SIMPLEWORK_CORE_NAMESPACE;
SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor)
    
    //
    // 数据类型定义
    //
    enum DataType {
        dtNull     = 0,
        dtBool     = 101,
        dtChar     = 102,
        dtInt      = 104,
        dtLong     = 105,
        dtFloat    = 201,
        dtDouble   = 202,
    };

    template<typename T> static DataType getDataType() {
        static DataType s_eRawType = getRawType<T>();
        return s_eRawType;
    };

    template<typename T> static DataType getRawType() {
        if(typeid(T) == typeid(bool)) {
            return dtBool;
        } else if(typeid(T) == typeid(char)) {
            return dtChar;
        } else if(typeid(T) == typeid(int)) {
            return dtInt;
        } else if(typeid(T) == typeid(long)) {
            return dtLong;
        } else if(typeid(T) == typeid(float)) {
            return dtFloat;
        } else if(typeid(T) == typeid(double)) {
            return dtDouble;
        }
        return dtNull;
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ITensor", 211202)
        //
        // 初始化张量，如果已经有数据，则原有数据将被删除
        // @eDt 张量数据类型
        // @nDimCnt 张量维度
        // @pDimSizes 张量每一个维度上的尺寸
        // @pData 张良原始数据值, nullptr表示数据不用初始化
        // @return
        //      0 == success
        //
        virtual int initTensor( DataType eDt, int nDim, int* pDimSizes, int nData, void* pData = nullptr) = 0;

        //
        // 获取数据类型
        //
        virtual DataType getDataType() = 0;

        //
        // 获取维度数量
        //
        virtual int getDimSize() = 0;

        //
        // 获取维度指针
        //
        virtual int* getDimPtr(int iPos = 0) = 0;

        //
        // 获取数据数量
        //
        virtual int getDataSize() = 0;

        //
        // 获取数据指针
        // @iPos 数据在数组中的位置
        // @pPosArr 数据在张量中的位置
        //
        virtual void* getDataPtr(int iPos = 0) = 0;
        virtual void* getDataPtr(int* pPosArr = nullptr) = 0;

        //
        // 截取
        // @pSubDimArr 选择维度和对应的子张量的开始位置及长度
        //      int pSubPosArr[][3] = {
        //             { iSubDim, iStartPos, nLength },
        //             { iSubDim, iStartPos, nLength },
        //              ...
        //             { iSubDim, iStartPos, nLength } };
        // @remark 注意返回的子张量维度与原有张量维度一样，维度顺序一样，只是维度尺寸为新要求尺寸
        //
        virtual Tensor subTensor(int nDim, int pSubDimArr[][3]) = 0;

        //
        // 连接
        // @iDim 连接的第几个维度
        //
        virtual Tensor catTensor(int iDim, Tensor& pTensor) = 0;

        
        //
        // 截取降维
        // @pReduceDimArr 需要降维的维度及取值
        //      int pReduceDimArr[][2] = {
        //              {iReduceDim, iPos},
        //              {iReduceDim, iPos},
        //              ...
        //              {iReduceDim, iPos} };
        //
        virtual Tensor reduceTensor(int nReduceDim, int pReduceDimArr[][2]) = 0;

        //
        // 连接升维
        // @pRaiseDimArr 需要降维的维度及取值
        //      int pRaiseDimArr[][2] = {
        //              {iRaiseDim, nLength},
        //              {iRaiseDim, nLength},
        //              ...
        //              {iRaiseDim, nLength} };
        //
        virtual Tensor raiseTensor(int nRaiseDim, int pRaiseDimArr[][2], Tensor* pRaiseTensorArray) = 0;

        //
        // 转置张量
        // @nTransposeDim 需要转置的维度数量
        // @pTransposeDimArr 需要转置的维度位置对照表
        //      int pTransposeDimArr[][2] = {
        //              {iTargetDim, iSourcceDim},
        //              {iTargetDim, iSourcceDim},
        //              ...
        //              {iTargetDim, iSourcceDim} };
        //      }
        //
        virtual Tensor transposeTensor(int nTransposeDim, int pTransposeDimArr[][2]) = 0;

    SIMPLEWORK_OBJECT_INTERFACE_LEAVE
    /*
    //
    // 构造张量
    //
    template<typename T> Tensor(int nData, T* pData, int nDim = 1, int* pDimSizeArr = nullptr) {
        *this = getFactory()->createObject();
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
    */
SIMPLEWORK_INTERFACE_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__