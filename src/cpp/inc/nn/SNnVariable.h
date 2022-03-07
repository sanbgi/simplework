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
    SNnVariable sigmoid() const { return solveOp("sigmoid"); }
    SNnVariable softmax() const { return solveOp("softmax"); }
    SNnVariable gap() const { return solveOp("gap"); }
    SNnVariable gmp() const { return solveOp("gmp"); }
    SNnVariable join(const SNnVariable& sp) const {
        return solveOp("join", *this, sp);
    }
    SNnVariable dense(const PNnDense& rDense) const {
        return solveOp("dense", *this, (const PArg*)CArg<PNnDense>(rDense));
    }
    SNnVariable maxpool(const PNnPool& rPool) const {
        return solveOp("maxpool", *this, (const PArg*)CArg<PNnPool>(rPool));
    }
    SNnVariable avgpool(const PNnPool& rPool) const {
        return solveOp("avgpool", *this, (const PArg*)CArg<PNnPool>(rPool));
    }
    SNnVariable conv(const PNnConv& rConv) const {
        return solveOp("conv", *this, (const PArg*)CArg<PNnConv>(rConv));
    }
    SNnVariable linear(const PNnLinear& rData) const {
        return solveOp("linear", *this, (const PArg*)CArg<PNnLinear>(rData));
    }
    SNnVariable batchNormalize(const PNnBatchNormalize& rNormalize) const {
        return solveOp("batchnormalize", *this, (const PArg*)CArg<PNnBatchNormalize>(rNormalize));
    }
    SNnVariable rnn(const PNnRnn& rData) const {
        return solveOp("rnn", *this, (const PArg*)CArg<PNnRnn>(rData));
    }
    SNnVariable gru(const PNnRnn& rData) const {
        return solveOp("gru", *this, (const PArg*)CArg<PNnRnn>(rData));
    }

public:
    static SNnVariable product(const SNnVariable& spVec, const SNnVariable& spMat) {
        SNnVariable o, in[2] = { spVec, spMat };
        SNnVariableSolver::getSolver()->solveOp("product", nullptr, 2, in, o);
        return o;
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
        return SObject::createObject("sw.nn.Weight", CArg<PNnWeight>(rWeight));
    }
    static SNnVariable solveOp(const char* szOp, const SNnVariable& a, const PArg* pData = nullptr) {
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
    static int solveOp(const char* szOperator, const PArg* pData,  int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar) {
        return SNnVariableSolver::getSolver()->solveOp(szOperator, pData, nInVars, pInVars, spOutVar);
    }


SIMPLEWORK_INTERFACECLASS_LEAVE(NnVariable)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnVariable_h__