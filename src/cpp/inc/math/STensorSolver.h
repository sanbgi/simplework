#ifndef __SimpleWork_STensorSolver_h__
#define __SimpleWork_STensorSolver_h__

#include "math.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(TensorSolver, "sw.math.TensorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorSolver", 211202)

        //
        // 张量相减
        //
        virtual int minus( const STensor& t1, const STensor& t2, STensor& spOut) = 0;

        virtual int multiply( const PTensor& t1, const PTensor& t2, IVisitor<const PTensor&>* pRecerver) = 0; 

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static STensorSolver& getSolver() {
        static STensorSolver g_solver = SObject::createObject<STensorSolver>();
        return g_solver;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(TensorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_STensorSolver_h__