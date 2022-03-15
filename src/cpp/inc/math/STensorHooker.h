#ifndef __SimpleWork_Math_STensorHooker_h__
#define __SimpleWork_Math_STensorHooker_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorHooker, "sw.math.TensorHooker")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorHooker", 220309)

        //
        // 内核求解(可以HOOK)
        //
        virtual int onSolve(
                        PRuntimeKey kernalKey, 
                        PMemory kernalParameter,
                        int nRanges, int pRanges[], 
                        int nVars, STensor pVars[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorHooker)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_STensorHooker_h__