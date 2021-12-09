#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

using namespace SIMPLEWORK_CORE_NAMESPACE;
SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor)

    SIMPLEWORK_INTERFACE_ENTER(IVector, "sw.math.ITensor", 211202)

    private:
        //
        // 初始化张量
        //
        virtual int initTensor( const Vector& spDimVector, Data::DataType eElementType, int nElementSize, void* pElementData = nullptr ) = 0;
        virtual int initTensor( const Vector& spDimVector, const Vector& spDataVector) = 0;

    public:
        //
        // 获取维度
        //
        virtual const Vector& getDimVector() = 0;

    public:
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
        //virtual Tensor subTensor(int nDim, int pSubDimArr[][3]) = 0;

        //
        // 连接
        // @iDim 连接的第几个维度
        //
        //virtual Tensor catTensor(int iDim, Tensor& pTensor) = 0;

        
        //
        // 截取降维
        // @pReduceDimArr 需要降维的维度及取值
        //      int pReduceDimArr[][2] = {
        //              {iReduceDim, iPos},
        //              {iReduceDim, iPos},
        //              ...
        //              {iReduceDim, iPos} };
        //
        //virtual Tensor reduceTensor(int nReduceDim, int pReduceDimArr[][2]) = 0;

        //
        // 连接升维
        // @pRaiseDimArr 需要降维的维度及取值
        //      int pRaiseDimArr[][2] = {
        //              {iRaiseDim, nLength},
        //              {iRaiseDim, nLength},
        //              ...
        //              {iRaiseDim, nLength} };
        //
        //virtual Tensor raiseTensor(int nRaiseDim, int pRaiseDimArr[][2], Vector* pRaiseTensorArray) = 0;

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
        //virtual Tensor transposeTensor(int nTransposeDim, int pTransposeDimArr[][2]) = 0;
        friend Tensor;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename Q=void> static Tensor createTensor(Vector& spDimVector, int nElementSize, Q* pElementData=nullptr) {
        Tensor tensor = createTensor();
        if(tensor) {
            tensor->initTensor(spDimVector, Data::getType<Q>(), nElementSize, (void*)pElementData);
        }
        return tensor;
    }
    static Tensor createTensor(const Vector& spDimVector, const Vector& spDataVector) {
        Tensor tensor = createTensor();
        if(tensor) {
            tensor->initTensor(spDimVector, spDataVector);
        }
        return tensor;
    }


private:
    static Tensor createTensor() {
        static Factory g_factory = Object::createFactory("sw.math.Tensor");
        return g_factory->createObject();
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__