#ifndef __SimpleWork_Core_SOperator_h__
#define __SimpleWork_Core_SOperator_h__

#include "core.h"
#include "PMemory.h"

__SimpleWork_Core_Namespace_Enter__

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Operator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IOperator", 220307)

        //
        // 运算
        //
        virtual int process(int nArgs, PMemory pArgs[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Operator)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SOperator_h__