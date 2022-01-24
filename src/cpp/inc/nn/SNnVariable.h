#ifndef __SimpleWork_NnVariable_h__
#define __SimpleWork_NnVariable_h__

#include "nn.h"
#include "SNnFactory.h"
#include "SNnVariableSolver.h"

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

    SNnVariable operator + (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("plus", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator += (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("plus", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator - (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("minus", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator -= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("minus", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator * (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("multiply", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator *= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("multiply", 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator / (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("divide", 2, pIns, o);
        return o;
    }

    const SNnVariable& operator /= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("divide", 2, pIns, *this);
        return (*this);
    }

    SNnVariable op(const char* szOp) {
        SNnVariable o;
        SNnVariableSolver::getSolver()->solveOp(szOp, 1, this, o);
        return o;
    }

    SNnVariable product(const SNnVariable& mat) {
        SNnVariable o, in[2] = { *this, mat };
        SNnVariableSolver::getSolver()->solveOp("product", 2, in, o);
        return o;
    }

    SDimension dimension() const{
        SDimension spDimension;
        IFace* pFace = getPtr();
        if(pFace) {
            pFace->getDimension(spDimension);
        }
        return spDimension;
    }

public:
    static int createState(int nDims, const int pDimSizes[], SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->createStateVariable(SDimension(nDims, pDimSizes), spVar);
    }

    static int createWeight(int nDims, const int pDimSizes[], SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->createWeightVariable(SDimension(nDims, pDimSizes), spVar);
    }    
    
    static int createState(const SDimension& spDim, SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->createStateVariable(spDim, spVar);
    }

    static int createWeight(const SDimension& spDim, SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->createWeightVariable(spDim, spVar);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(NnVariable)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnVariable_h__