#ifndef __SimpleWork_NnEctx_h__
#define __SimpleWork_NnEctx_h__

#include "nn.h"
#include "SNnFactory.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

class SNnVariable;

//
// 神经网络单元求解上下文（当神经网络单元为多张量输出时启用）
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnEctx)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnEctx", 220204)

        //
        // 输出变量
        //
        virtual int outVar(int nOutVars, const SNnVariable pOutVars[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnEctx)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnEctx_h__