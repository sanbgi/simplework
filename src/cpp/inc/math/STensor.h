#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include "SMathFactory.h"
#include "SDimension.h"
#include "STensorEquation.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class SDevice;

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(Tensor, "sw.math.Tensor")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensor", 211202)
    
        //
        // 获取张量维度信息
        //
        virtual int getDimension(SDimension& spDim) = 0;

        //
        // 获取元素类型
        //
        virtual PDATATYPE getDataType() = 0;

        //
        // 获取元素数量
        //
        virtual int getDataSize() = 0;

        //
        // 转移到指定设备，并返回数据
        //
        virtual int toDevice(const SDevice& spDevice, PVector* pDeviceData) = 0;

        //
        // 获取元素数据指针
        //
        virtual void* getDataPtr(PDATATYPE eElementType, int iPos=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename Q> static STensor createValue(Q v) {
        STensor spTensor;
        SMathFactory::getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), 1, (void*)&v);
        return spTensor;
    }

    //
    // 构造一维张量
    //
    static int createVector(STensor& spTensor, PDATATYPE idElementType, int nElementSize, const void* pElementData=nullptr) {
        return SMathFactory::getFactory()->createVector(spTensor, idElementType, nElementSize, pElementData);
    }
    template<typename Q> static STensor createVector(int nElementSize, Q* pElementData=nullptr) {
        STensor spTensor;
        SMathFactory::getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
        return spTensor;
    }
    template<typename Q> static int createVector(STensor& spTensor, int nElementSize, const Q* pElementData=nullptr) {
        return SMathFactory::getFactory()->createVector(spTensor, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }
    //
    // 构造多维张量
    //
    static int createTensor(STensor& spTensor, const SDimension& spDimVector, PDATATYPE iElementType, int nElementSize, const void* pElementData=nullptr) {
        return SMathFactory::getFactory()->createTensor(spTensor, spDimVector, iElementType, nElementSize, (void*)pElementData);
    }
    template<typename Q> static int createTensor(STensor& spTensor, const SDimension& spDimVector, int nElementSize, const Q* pElementData=nullptr) {
        return SMathFactory::getFactory()->createTensor(spTensor, spDimVector, CBasicData<Q>::getStaticType(), nElementSize, (void*)pElementData);
    }


public:
    int size() const{
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getDataSize() : 0;
    }

    void* data(int iPos=0) const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getDataPtr(pFace->getDataType(), iPos) : nullptr;
    }

    template<typename Q> Q* data(int iPos = 0) const{
        IFace* pFace = getPtr();
        return pFace != nullptr ? (Q*)pFace->getDataPtr(pFace->getDataType(), iPos) : (Q*)nullptr;
    }

    SDimension dimension() const {
        SDimension spDim;
        IFace* pFace = getPtr();
        if(pFace != nullptr) {
            pFace->getDimension(spDim);
        }
        return spDim;
    }

    PDATATYPE type() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getDataType() : 0;
    }

public:
    static STensor eval(STensorEquation spEquation, const STensor& a) {
        STensor spOut;
        spEquation->eval(nullptr, 1, &a, spOut);
        return spOut;
    }
    static STensor eval(STensorEquation spEquation, const STensor& a, const STensor& b) {
        STensor spOut;
        STensor pInVars[] = { a, b };
        spEquation->eval(nullptr, 2, pInVars, spOut);
        return spOut;
    }
    STensor operator + (const STensor& spIn) {
        static STensorEquation spEquation = SObject::createObject("sw.math.PlusEquation");
        return eval(spEquation, *this, spIn);
    }
    STensor operator - (const STensor& spIn) {
        static STensorEquation spEquation = SObject::createObject("sw.math.MinusEquation");
        return eval(spEquation, *this, spIn);
    }

    //
    // 均方根
    //
    STensor rootMeanSquare () {
        static STensorEquation spEquation = SObject::createObject("sw.math.RootMeanSquareEquation");
        return eval(spEquation, *this);
    }
    
SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__