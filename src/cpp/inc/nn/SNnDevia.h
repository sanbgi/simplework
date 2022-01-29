#ifndef __SimpleWork_NnDevia_h__
#define __SimpleWork_NnDevia_h__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER

//
// 神经网络权重偏差定义，用于更新神经网络权重(未来并行训练时启用)
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnDevia, "sw.nn.NnDevia")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnDevia", 211202)


    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnDevia)

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NnDevia_h__