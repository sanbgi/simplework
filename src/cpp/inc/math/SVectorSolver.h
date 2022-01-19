#ifndef __SimpleWork_SVectorSolver_h__
#define __SimpleWork_SVectorSolver_h__

#include "math.h"
#include "SMathFactory.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(VectorSolver, "sw.math.VectorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IVectorSolver", 211202)
        //
        // 将一个向量加入另外一个向量或减去
        //
        virtual int add(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int del(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int copy(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int zero(int nSize, void* pDesc) = 0;

        //
        // 两个向量相加及反向偏导（累加模式）
        //
        virtual int add(const PVectorArray& inVars, const PVector& outVar) = 0;
        virtual int addAccDeviation(const PVector& outDVar, const PVectorArray& inDVars) = 0;

        //
        // 向量矩阵相乘及反向偏导（累加模式）
        //
        virtual int multiply(const PVectorArray& inVars, const PVector& outVar) = 0;
        virtual int multiplyAccDeviation(const PVector& outDVar, const PVectorArray& inDVars, const PVectorArray& inVars) = 0;

        //
        // 两个向量连接及反向偏导（累加模式）
        //
        virtual int join(const PVectorArray& inVars, const PVector& outVar) = 0;
        virtual int joinAccDeviation(const PVector& outDVar, const PVectorArray& inDVars) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static int getSolver(unsigned int idType, SVectorSolver& spSolver) {
        return SMathFactory::getFactory()->createSolver(idType, spSolver);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(VectorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SVectorSolver_h__