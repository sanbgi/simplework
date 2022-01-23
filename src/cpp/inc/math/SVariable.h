#ifndef __SimpleWork_Variable_h__
#define __SimpleWork_Variable_h__

#include "PVariable.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 抽象变量定义，变量可以表示任何可运算的对象，包括：值/向量/矩阵/张量/单位等等
//
SIMPLEWORK_INTERFACECLASS_ENTER(Variable, "sw.math.Variable")

    enum EOperator {
        NONE,
        PLUS,
        MINUS,
        MULTIPLY,
        DIVIDE,
    };

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IVariable", 211202)

        virtual int eval(EOperator eOp, int nInVars, const SVariable pInVars[], SVariable& outVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SVariable operator + (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(PLUS, 1, &b, o);
        }
        return o;
    }

    const SVariable& operator += (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(PLUS, 1, &b, (*this));
        }
        return (*this);
    }

    SVariable operator - (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(MINUS, 1, &b, o);
        }
        return o;
    }

    const SVariable& operator -= (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(MINUS, 1, &b, (*this));
        }
        return (*this);
    }

    SVariable operator * (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(MULTIPLY, 1, &b, o);
        }
        return o;
    }

    const SVariable& operator *= (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(MULTIPLY, 1, &b, (*this));
        }
        return (*this);
    }

    SVariable operator / (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(DIVIDE, 1, &b, o);
        }
        return o;
    }

    const SVariable& operator /= (const SVariable& b) {
        SVariable o;
        IFace* pOperator = getPtr();
        if(pOperator) {
            pOperator->eval(DIVIDE, 1, &b, (*this));
        }
        return (*this);
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(Variable)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Variable_h__