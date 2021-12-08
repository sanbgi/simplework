#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

using namespace SIMPLEWORK_CORE_NAMESPACE;
SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.ITensor", 211202)
        //
        // 初始化张量，如果已经有数据，则原有数据将被删除
        // @eDt 张量数据类型
        // @nDimCnt 张量维度
        // @pDimSizes 张量每一个维度上的尺寸
        // @pData 张良原始数据值, nullptr表示数据不用初始化
        // @return
        //      0 == success
        //
        virtual int initTensor( EBasicType eDt, int nDim, int* pDimSizes, int nData, void* pData = nullptr) = 0;

        //
        // 获取数据类型
        //
        virtual EBasicType getDataType() = 0;

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

    SIMPLEWORK_INTERFACE_LEAVE
    /*
    //
    // 构造张量
    //
    template<typename T> Tensor(int nData, T* pData, int nDim = 1, int* pDimSizeArr = nullptr) {
        *this = getFactory()->createObject();
        getPtr()->initTensor(ITensor::getDataType<T>(), nDim, pDimSizeArr, nData, pData);
    };
    */

SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__