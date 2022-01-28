#ifndef __SimpleWork_STensorSolver_h__
#define __SimpleWork_STensorSolver_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;
class SDimension;

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(TensorSolver, "sw.math.TensorSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorSolver", 211202)

        //
        // 张量相减
        //
        virtual int minus( const STensor& t1, const STensor& t2, STensor& spOut) = 0;

        //
        // 升维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int upHighDimension(const SDimension& spIn, int nDimSize, SDimension& spOut) = 0;

        //
        // 降维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int downHighDimension(const SDimension& spIn, SDimension& spOut) = 0;

        //
        // 升维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int upLowDimension(const SDimension& spIn, int nDimSize, SDimension& spOut) = 0;

        //
        // 降维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int downLowDimension(const SDimension& spIn, SDimension& spOut) = 0;

        //
        // 判断维度是否相同
        //
        virtual bool isEqual(const SDimension& spDim1, const SDimension& spDim2) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static STensorSolver& getSolver() {
        static STensorSolver g_solver = SObject::createObject<STensorSolver>();
        return g_solver;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(TensorSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_STensorSolver_h__