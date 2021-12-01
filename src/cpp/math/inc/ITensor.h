#ifndef __SimpleWork_ITensor_h__
#define __SimpleWork_ITensor_h__

#include <typeinfo>

namespace sw::tensor {

/**
 * 所有对象的接口
 */
SIMPLEWORK_INTERFACE_ENTER(ITensor, sw::core::IObject, "sw.math.ITensor", 011130)

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

    //
    // 初始化张量，如果已经有数据，则原有数据将被删除
    // @eDt 张量数据类型
    // @nDimCnt 张量维度
    // @pDimSizes 张量每一个维度上的尺寸
    // @pData 张良原始数据值, nullptr表示数据不用初始化
    // @return
    //      0 == success
    //
    virtual int initTensor( ITensor::DataType eDt, int nDim, int* pDimSizes, int nData, void* pData = nullptr) = 0;

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
    virtual int subTensor(int nDim, int pSubDimArr[][3], ITensor** ppSubTensor) = 0;

    //
    // 连接
    // @iDim 连接的第几个维度
    //
    virtual int catTensor(int iDim, ITensor* pTensor, ITensor** ppCatTensor) = 0;

    
    //
    // 截取降维
    // @pReduceDimArr 需要降维的维度及取值
    //      int pReduceDimArr[][2] = {
    //              {iReduceDim, iPos},
    //              {iReduceDim, iPos},
    //              ...
    //              {iReduceDim, iPos} };
    //
    virtual int reduceTensor(int nReduceDim, int pReduceDimArr[][2], ITensor** ppReduceTensor) = 0;

    //
    // 连接升维
    // @pRaiseDimArr 需要降维的维度及取值
    //      int pRaiseDimArr[][2] = {
    //              {iRaiseDim, nLength},
    //              {iRaiseDim, nLength},
    //              ...
    //              {iRaiseDim, nLength} };
    //
    virtual int raiseTensor(int nRaiseDim, int pRaiseDimArr[][2], ITensor** ppRaiseTensorArray, ITensor* ppRaiseTensor) = 0;

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
    virtual int transposeTensor(int nTransposeDim, int pTransposeDimArr[][2], ITensor** ppTransposeTensor) = 0;

SIMPLEWORK_INTERFACE_LEAVE//Tensor

}//namespace SimpleWork

#endif//__SimpleWork_ITensor_h__