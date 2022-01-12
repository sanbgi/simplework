#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include "STensorSolver.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor, "sw.math.Tensor")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensor", 211202)

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

    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename Q> static STensor createValue(Q v) {
        STensor spTensor;
        STensorSolver::getSolver()->createVector(spTensor, CBasicData<Q>::getStaticType(), 1, (void*)&v);
        return spTensor;
    }
    template<typename Q> static STensor createVector(int nElementSize, Q* pElementData=nullptr) {
        STensor spTensor;
        STensorSolver::getSolver()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
        return spTensor;
    }
    //
    // 构造一维张量
    //
    template<typename Q> static int createVector(STensor& spTensor, int nElementSize, Q* pElementData=nullptr) {
        return STensorSolver::getSolver()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }
    //
    // 构造多维张量
    //
    template<typename Q> static int createTensor(STensor& spTensor, STensor& spDimVector, int nElementSize, Q* pElementData=nullptr) {
        return STensorSolver::getSolver()->createTensor(spTensor, spDimVector, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }
    static int createTensor(STensor& spTensor, STensor& spDimVector, unsigned int iElementType, int nElementSize, void* pElementData=nullptr) {
        return STensorSolver::getSolver()->createTensor(spTensor, spDimVector, iElementType, nElementSize, (void*)pElementData);
    }

public:
    STensor operator - (const STensor& spIn) {
        STensor spOut;
        STensorSolver::getSolver()->minus(*this, spIn, spOut);
        return spOut;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__