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

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnVariable", 211202)

        //
        // 获取变量维度
        //
        virtual int getDimension(SDimension& spDimension) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SNnVariable operator + (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("plus", nullptr, 2, pIns, o);
        return o;
    }

    const SNnVariable& operator += (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("plus", nullptr, 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator - (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("minus", nullptr, 2, pIns, o);
        return o;
    }

    const SNnVariable& operator -= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("minus", nullptr, 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator * (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("multiply", nullptr, 2, pIns, o);
        return o;
    }

    const SNnVariable& operator *= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("multiply", nullptr, 2, pIns, *this);
        return (*this);
    }

    SNnVariable operator / (const SNnVariable& b) const {
        SNnVariable o, pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("divide", nullptr, 2, pIns, o);
        return o;
    }

    const SNnVariable& operator /= (const SNnVariable& b) {
        SNnVariable pIns[] = {*this, b};
        SNnVariableSolver::getSolver()->solveOp("divide", nullptr, 2, pIns, *this);
        return (*this);
    }

    SNnVariable solveOp(const char* szOp) const{
        SNnVariable o;
        SNnVariableSolver::getSolver()->solveOp(szOp, nullptr, 1, this, o);
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
    SNnVariable relu() const { return solveOp("relu"); }
    SNnVariable tanh() const { return solveOp("tanh"); }
    SNnVariable sigmod() const { return solveOp("sigmod"); }
    SNnVariable softmax() const { return solveOp("softmax"); }
    SNnVariable gap() const { return solveOp("gap"); }
    SNnVariable dense(const PNnDense& rDense) const {
        return solveOp("dense", *this, (const PData*)CData<PNnDense>(rDense));
    }
    SNnVariable pool(const PNnPool& rPool) const {
        return solveOp("pool", *this, (const PData*)CData<PNnPool>(rPool));
    }
    SNnVariable conv(const PNnConv& rConv) const {
        return solveOp("conv", *this, (const PData*)CData<PNnConv>(rConv));
    }
    SNnVariable linear(const PNnLinear& rData) const {
        return solveOp("linear", *this, (const PData*)CData<PNnLinear>(rData));
    }
    SNnVariable batchNormalize(const PNnBatchNormalize& rNormalize) const {
        return solveOp("batchnormalize", *this, (const PData*)CData<PNnBatchNormalize>(rNormalize));
    }
    SNnVariable rnn(const PNnRnn& rData) const {
        return solveOp("rnn", *this, (const PData*)CData<PNnRnn>(rData));
    }
    SNnVariable gru(const PNnRnn& rData) const {
        return solveOp("gru", *this, (const PData*)CData<PNnRnn>(rData));
    }

public:
    static SNnVariable product(const SNnVariable& spVec, const SNnVariable& spMat) {
        SNnVariable o, in[2] = { spVec, spMat };
        SNnVariableSolver::getSolver()->solveOp("product", nullptr, 2, in, o);
        return o;
    }
    static int createState(const SDimension& spDim, SNnState& spVar) {
        return SNnVariableSolver::getSolver()->createState(spDim, spVar);
    }
    static int loadState(const SNnState spState, SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->loadState(spState, spVar);
    }
    static SNnVariable loadState(const SNnState spState) {
        SNnVariable o;
        SNnVariableSolver::getSolver()->loadState(spState, o);
        return o;
    }
    static int saveState(const SNnState spState, const SNnVariable& spVar) {
        return SNnVariableSolver::getSolver()->saveState(spState, spVar);
    }
    static SNnVariable createWeight(const PNnWeight& rWeight) {
        return SObject::createObject("sw.nn.Weight", CData<PNnWeight>(rWeight));
    }
    static SNnVariable solveOp(const char* szOp, const SNnVariable& a, const PData* pData = nullptr) {
        SNnVariable o;
        SNnVariableSolver::getSolver()->solveOp(szOp, pData, 1, &a, o);
        return o;
    }
    static SNnVariable solveOp(const char* szOp, const SNnVariable& a, const SNnVariable& b) {
        SNnVariable o;
        SNnVariable pInVars[2] = {a, b};
        SNnVariableSolver::getSolver()->solveOp(szOp, nullptr, 2, pInVars, o);
        return o;
    }
    static int solveOp(const char* szOperator, const PData* pData,  int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
        return SNnVariableSolver::getSolver()->solveOp(szOperator, pData, nInVars, pInVars, spOutVar);
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(NnVariable)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnVariable_h__