#ifndef __SimpleWork_SMathSolver_h__
#define __SimpleWork_SMathSolver_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(MathSolver, "sw.math.MathSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IMathSolver", 211202)

        //
        // 张量相减
        //
        virtual int minus( const STensor& t1, const STensor& t2, STensor& spOut) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SMathSolver& getSolver() {
        static SMathSolver g_solver = SObject::createObject<SMathSolver>();
        return g_solver;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(MathSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SMathSolver_h__