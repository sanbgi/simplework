#ifndef __SimpleWork_SDimensionSolver_h__
#define __SimpleWork_SDimensionSolver_h__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;
class SDimension;

/**
 * 张量类定义
 */
SIMPLEWORK_INTERFACECLASS_ENTER(DimensionSolver, "sw.math.DimensionSolver")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IDimensionSolver", 211202)

        //
        // 升维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int upHighDimension(const SDimension& spIn, int nDims, int pDimSizes[], SDimension& spOut) = 0;

        //
        // 降维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int downHighDimension(const SDimension& spIn, int nDims, SDimension& spOut) = 0;

        //
        // 升维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int upLowDimension(const SDimension& spIn, int nDims, int pDimSizes[], SDimension& spOut) = 0;

        //
        // 降维
        //  @nDimSize 第一个维度(新维度)大小
        //
        virtual int downLowDimension(const SDimension& spIn, int nDims, SDimension& spOut) = 0;

        //
        // 判断维度是否相同
        //
        virtual bool isEqual(const SDimension& spDim1, const SDimension& spDim2) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SDimensionSolver& getSolver() {
        static SDimensionSolver g_solver = SObject::createObject<SDimensionSolver>();
        return g_solver;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(DimensionSolver)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SDimensionSolver_h__