#ifndef __SimpleWork_NnUnit_h__
#define __SimpleWork_NnUnit_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnVariable;

//
// 神经网络单元
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnUnit, "sw.nn.NnUnit")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnUnit", 211202)

        //
        // 获取变量维度
        //
        virtual int eval(const SNnVariable& spIn, SNnVariable& spOut) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnUnit)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnUnit_h__