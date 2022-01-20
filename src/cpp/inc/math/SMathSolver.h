#ifndef __SimpleWork_SMathSolver_h__
#define __SimpleWork_SMathSolver_h__

#include "math.h"
#include "SMathFactory.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 向量求解器
//
//  注意：不同数据类型的向量求解器，不能混用，为了性能，系统并未检查数据类型
//
SIMPLEWORK_INTERFACECLASS_ENTER(MathSolver, "sw.math.MathSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IMathSolver", 211202)
        //
        // 将一个向量加入另外一个向量或减去
        //
        virtual int add(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int del(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int copy(int nSize, void* pDesc, void* pSrc) = 0;
        virtual int zero(int nSize, void* pDesc) = 0;
        virtual int add(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;
        virtual int del(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;
        //
        //  加权求和，y = in1 * weight + in2 * (1-weight)
        //
        virtual int addByWeight(int nSize, void* pIn1, void* pIn2, void* pWeight, void* pOut) = 0;
        virtual int multiply(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;

        //
        // 向量（输入） * 矩阵 = 向量（输出）
        //
        virtual int multiply(PVector vecIn, PVector vecMatrix, PVector vecOut) = 0;
        virtual int multiplyAccDeviation(PDeviaVector vecOut, PDeviaVector vecWeights, PDeviaVector vecIn) = 0;

        //
        // 两个向量连接
        //
        virtual int join(PVector vecIn1, PVector vecIn2, PVector vecOut) = 0;
  
    SIMPLEWORK_INTERFACE_LEAVE

    static int getSolver(unsigned int idType, SMathSolver& spSolver) {
        return SMathFactory::getFactory()->createSolver(idType, spSolver);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(MathSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SMathSolver_h__