#ifndef __SimpleWork_TensorEquation_h__
#define __SimpleWork_TensorEquation_h__

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 操作符定义
//
SIMPLEWORK_INTERFACECLASS_ENTER(TensorEquation, "sw.math.TensorEquation")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ITensorEquation", 211202)

        //
        // 求值
        //
        virtual int eval(const PData* pData, int nInVars, const STensor pInVars[], STensor& outVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(TensorEquation)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_TensorEquation_h__