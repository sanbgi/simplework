#ifndef __SimpleWork_SNeuralPipe_h__
#define __SimpleWork_SNeuralPipe_h__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络核心接口定义
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NeuralPipe)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.math.INeuralPipe", 220105)

        //
        // 向管道注入数据
        //      @spInTensor 输入张量
        //      @spOutTensor 输出张量
        //
        virtual int push(const sw::math::STensor& spInTensor, sw::math::STensor& spOutTensor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NeuralPipe)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_SNeuralPipe_h__