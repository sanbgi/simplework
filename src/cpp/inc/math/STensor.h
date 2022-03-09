#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include "SMathFactory.h"
#include "SDimension.h"
#include "STensorSolver.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

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
        // 返回元素数据
        //
        virtual int getDataInDevice(const SDevice& spDevice, PVector& deviceData) = 0;
        
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

    void* data(const SDevice& spDevice=SDevice::cpu()) const {
        PVector deviceMemory = { 0, nullptr };
        IFace* pFace = getPtr();
        if(pFace != nullptr) {
            pFace->getDataInDevice(SDevice::cpu(), deviceMemory); 
        }
        return deviceMemory.data;
    }

    template<typename Q> Q* data(const SDevice& spDevice=SDevice::cpu()) const{
        PVector deviceMemory = { 0, nullptr };
        IFace* pFace = getPtr();
        if(pFace != nullptr) {
            pFace->getDataInDevice(SDevice::cpu(), deviceMemory); 
        }
        return (Q*)deviceMemory.data;
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
    static int solveOp(const POperator& spOp, int nVars, STensor pVars[]) {
        static STensorSolver spOperator = SObject::createObject("sw.math.TensorSolver");
        return spOperator->solve(spOp, nVars, pVars);
    }

public:
    STensor avg() {
        STensor pVars[2] = {*this};
        solveOp({POperator::avg}, 2, pVars);
        return pVars[1];
    }
    STensor sqrt() {
        STensor pVars[2] = {*this};
        solveOp({POperator::sqrt}, 2, pVars);
        return pVars[1];
    }
    STensor square() {
        STensor pVars[2] = {*this};
        solveOp({POperator::square}, 2, pVars);
        return pVars[1];
    }
    STensor rootMeanSquare() {
        return square().avg().sqrt();
    }
    STensor variance(){
        return ((*this)-avg()).rootMeanSquare();
    }
    STensor operator + (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        solveOp({POperator::plus}, 3, pVars);
        return pVars[2];
    }
    STensor operator - (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        solveOp({POperator::minus}, 3, pVars);
        return pVars[2];
    }
    STensor operator * (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        solveOp({POperator::multiply}, 3, pVars);
        return pVars[2];
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__