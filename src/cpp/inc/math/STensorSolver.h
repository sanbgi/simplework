#ifndef __SimpleWork_STensorSolver_h__
#define __SimpleWork_STensorSolver_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensorHooker;
struct PTensorOperator {
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

        toFloat,
        toDouble,
    }id;
};

//
// 向量求解器
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorSolver, "sw.math.TensorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorSolver", 220309)
        //
        // 解析求解
        //
        virtual int solve( const PTensorOperator& rOp, int nVars, STensor pVars[]) = 0;

        //
        // 内核求解(可以HOOK)
        //
        virtual int solve(
                        PKernalKey kernalKey, 
                        PVector kernalRange, 
                        PMemory kernalParameter,
                        int nVars, STensor pVars[]) = 0;

        //
        // 设置钩子，只有最后一个钩子有效
        //
        virtual int pushHooker(const STensorHooker& spHooker) = 0;

        //
        // 获取钩子
        //
        virtual int getHooker(STensorHooker& spHooker) = 0;

        //
        // 取消钩子
        //
        virtual int popHooker() = 0;


    SIMPLEWORK_INTERFACE_LEAVE

    static STensorSolver getSolver(){
        static STensorSolver g_solver = SObject::createObject(STensorSolver::__getClassKey());
        return g_solver;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_STensorSolver_h__