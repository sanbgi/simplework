#ifndef __SimpleWork_Tensor_h__
#define __SimpleWork_Tensor_h__

#include "SMathFactory.h"
#include "SDimension.h"
#include "STensorSolver.h"
#include "STensorOperator.h"

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
        // 获取数据
        //  
        virtual int getDataBuffer(SDeviceMemory& spMemory) = 0;

        //
        // 获取元素类型
        //
        virtual PDATATYPE getDataType() = 0;

        //
        // 获取元素数量
        //
        virtual int getDataSize() = 0;

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
    static int createTensor(STensor& spTensor, PDATATYPE iElementType, int nElementSize, const SDimension& spDimension, const SDeviceMemory& spDataBuffer ) {
        return SMathFactory::getFactory()->createTensor(spTensor, iElementType, nElementSize, spDimension, spDataBuffer);
    }

public:
    int size() const{
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getDataSize() : 0;
    }

    void* data(const SDevice& spDevice=SDevice::cpu()) const {
        return dataBuffer().data(spDevice);
    }

    template<typename Q> Q* data(const SDevice& spDevice=SDevice::cpu()) const{
        return (Q*)dataBuffer().data(spDevice);
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

    SDevice device() const {
        return dataBuffer().device();
    }

    SDeviceMemory dataBuffer() const {
        SDeviceMemory spBuffer;
        IFace* pFace = getPtr();
        if(pFace != nullptr) { pFace->getDataBuffer(spBuffer); }
        return spBuffer;
    }

public:
    static STensor& solveOp(const PTensorOperator& spOp, int nVars, STensor pVars[]) {
        static STensorSolver spOperator = SObject::createObject("sw.math.TensorSolver");
        spOperator->solve(spOp, nVars, pVars);
        return pVars[nVars-1];
    }

public:
    STensor toFloat() {
        STensor pVars[2] = {*this};
        return solveOp({PTensorOperator::toFloat}, 2, pVars);
    }
    STensor toDouble() {
        STensor pVars[2] = {*this};
        return solveOp({PTensorOperator::toDouble}, 2, pVars);
    }
    STensor oneHot(int nClassify, PDATATYPE toType) {
        static STensorOperator spOperator = SObject::createObject("sw.math.TensorOneHot");
        STensor pVars[2] = {*this};
        spOperator->solve(CData<POneHot>({nClassify, toType}), 2, pVars);
        return pVars[1];
    }
    STensor avg() {
        STensor pVars[2] = {*this};
        return solveOp({PTensorOperator::avg}, 2, pVars);
    }
    STensor sqrt() {
        STensor pVars[2] = {*this};
        return solveOp({PTensorOperator::sqrt}, 2, pVars);
    }
    STensor square() {
        STensor pVars[2] = {*this};
        return solveOp({PTensorOperator::square}, 2, pVars);
    }
    STensor rootMeanSquare() {
        return square().avg().sqrt();
    }
    STensor variance(){
        return ((*this)-avg()).rootMeanSquare();
    }
    STensor operator + (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        return solveOp({PTensorOperator::plus}, 3, pVars);
    }
    STensor operator - (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        return solveOp({PTensorOperator::minus}, 3, pVars);
    }
    STensor operator * (const STensor& spIn) {
        STensor pVars[3] = {*this, spIn};
        return solveOp({PTensorOperator::multiply}, 3, pVars);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Tensor)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Tensor_h__