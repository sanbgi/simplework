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

        /*
        //
        // 单元运算
        //
        virtual void set(PVector vec, int value) = 0;

        //
        // 双元运算
        //
        virtual void copy(int nSize, void* pDesc, void* pSrc) = 0;
        virtual void plus(int nSize, void* pDesc, void* pSrc) = 0;
        virtual void minus(int nSize, void* pDesc, void* pSrc) = 0;

        //
        // 三元运算
        //
        virtual void plus(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;
        virtual void minus(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;
        virtual void multiply(int nSize, void* pIn1, void* pIn2, void* pOut) = 0;

        //
        // 向量（输入） * 矩阵 = 向量（输出）
        //
        virtual int product(PVector vecIn, PVector vecMatrix, PVector vecOut) = 0;

        //
        // 两个向量连接
        //
        virtual int join(PVector vecIn1, PVector vecIn2, PVector vecOut) = 0;
        */
  
    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(MathSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SMathSolver_h__