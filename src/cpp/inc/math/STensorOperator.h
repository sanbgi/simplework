#ifndef __SimpleWork_Math_STensorOperator_h__
#define __SimpleWork_Math_STensorOperator_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorOperator, "sw.math.TensorOperator")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorOperator", 220309)
        //
        // 解析求解
        //
        virtual int solve(const PData* pData, int nVars, STensor pVars[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorOperator)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_STensorOperator_h__