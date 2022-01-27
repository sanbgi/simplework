#ifndef __SimpleWork_Equation_h__
#define __SimpleWork_Equation_h__

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 操作符定义
//
SIMPLEWORK_INTERFACECLASS_ENTER(Equation, "sw.math.Equation")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.IEquation", 211202)

        //
        // 求值
        //
        virtual int eval(int nInVars, const SVariable pInVars[], SVariable& outVar) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(Equation)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Equation_h__