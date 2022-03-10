#ifndef __SimpleWork_STensorSolver_h__
#define __SimpleWork_STensorSolver_h__

#include "math.h"
#include "SMathFactory.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

struct POperator {
    enum {
        plus,
        minus,
        multiply,
        divide,
        product,
        square,
        sqrt,
        sum,
        avg,
    }id;
    const PData* extra;
};

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorSolver, "sw.math.TensorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorSolver", 220309)
        //
        // 解析求解
        //
        virtual int solve(
                        const POperator& sOp, 
                        int nVars, STensor pVars[]) = 0;

        //
        // 内核求解(可以HOOK)
        //
        virtual int solve(
                        PRuntimeKey kernalKey, 
                        PVector kernalRange, 
                        PMemory kernalParameter,
                        int nVars, STensor pVars[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static STensorSolver getSolver(){
        static STensorSolver g_solver = SObject::createObject(STensorSolver::__getClassKey());
        return g_solver;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_STensorSolver_h__