#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

using namespace SIMPLEWORK_CORE_NAMESPACE;

SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor, "sw.math.Tensor")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensor", 211202)

    public:
        //
        // 获取维度
        //
        virtual STensor& getDimVector() = 0;

        //
        // 获取元素类型
        //
        virtual unsigned int getDataType() = 0;

        //
        // 获取元素数量
        //
        virtual int getDataSize() = 0;

        //
        // 获取元素数据指针
        //
        virtual void* getDataPtr(unsigned int eElementType, int iPos=0) = 0;

    public:
        //
        // 获取元素数据指针
        //
        template<typename Q> inline Q* getDataPtr(int iPos=0) {
            return (Q*)getDataPtr(CBasicData<Q>::getStaticType(), iPos);
        }

        //
        // 获取元素值，不安全
        //
        template<typename Q> inline Q& getDataAt(int iPos) {
            return *getDataPtr<Q>(iPos);
        }

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
        friend STensor;
    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(TensorFactory, "sw.math.TensorFactory")
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorFactory", 211211)
            //
            // 构造全新的张量，私有函数，因为指针没类型，易出错
            //
            virtual int createVector(STensor& spTensor, unsigned int eElementType, int nElementSize, void* pElementData = nullptr) = 0;
            virtual int createTensor(STensor& spTensor, const STensor& spDimVector, unsigned int eElementType, int nElementSize, void* pElementData = nullptr ) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(TensorFactory)

public:
    template<typename Q> static STensor createValue(Q v) {
        STensor spTensor;
        getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), 1, (void*)&v);
        return spTensor;
    }
    template<typename Q> static STensor createVector(int nElementSize, Q* pElementData=nullptr) {
        STensor spTensor;
        getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
        return spTensor;
    }
    //
    // 构造一维张量
    //
    template<typename Q> static int createVector(STensor& spTensor, int nElementSize, Q* pElementData=nullptr) {
        return getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }
    //
    // 构造多维张量
    //
    template<typename Q> static int createTensor(STensor& spTensor, STensor& spDimVector, int nElementSize, Q* pElementData=nullptr) {
        return getFactory()->createTensor(spTensor, spDimVector, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }
    static int createTensor(STensor& spTensor, STensor& spDimVector, unsigned int iElementType, int nElementSize, void* pElementData=nullptr) {
        return getFactory()->createTensor(spTensor, spDimVector, iElementType, nElementSize, (void*)pElementData);
    }

private:
    static STensorFactory& getFactory() {
        static STensorFactory g_factory = SObject::createObject<STensorFactory>();
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__