#ifndef __SimpleWork_NnVariable_h__
#define __SimpleWork_NnVariable_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 抽象变量定义，变量可以表示任何可运算的对象，包括：值/向量/矩阵/张量/单位等等
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnVariable, "sw.nn.NnVariable")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnVariable", 211202)

        //
        // 获取变量维度
        //
        virtual int getDimension(SDimension& spDimension) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SNnVariable operator + (const SNnVariable& b) {
        SNnVariable o, pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("plus", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator += (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("plus", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator - (const SNnVariable& b) {
        SNnVariable o, pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("minus", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator -= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("minus", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator * (const SNnVariable& b) {
        SNnVariable o, pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("multiply", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator *= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("multiply", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator / (const SNnVariable& b) {
        SNnVariable o, pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("divide", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator /= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnFactory::getFactory()->createOpVariable("divide", 2, pIns, *this);
        return (*this);
    }

    SDimension dimension() const{
        SDimension spDimension;
        IFace* pFace = getPtr();
        if(pFace) {
            pFace->getDimension(spDimension);
        }
        return spDimension;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnVariable)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnVariable_h__