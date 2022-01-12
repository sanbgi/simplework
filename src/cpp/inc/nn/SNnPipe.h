#ifndef __SimpleWork_SNeuralPipe_h__
#define __SimpleWork_SNeuralPipe_h__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnPipe)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.math.INnPipe", 220105)

        //
        // 向管道注入数据
        //      @spInTensor 输入张量
        //      @spOutTensor 输出张量
        //
        virtual int push(const sw::STensor& spInTensor, sw::STensor& spOutTensor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnPipe)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_SNeuralPipe_h__