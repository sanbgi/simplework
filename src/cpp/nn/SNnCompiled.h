#ifndef __SimpleWork_NN_SNnCompiledUnit_H__
#define __SimpleWork_NN_SNnCompiledUnit_H__

#include "nn.h"

//
// 神经网络单元
//
SIMPLEWORK_INTERFACECLASS_ENTER(NnCompiledUnit, "sw.nn.NnCompiledUnit")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.INnCompiledUnit", 211202)

        //
        // 求解
        //
        virtual int eval(const STensor& spBatchIn, STensor& spBatchOut) = 0;

        //
        // 求偏差
        //
        virtual int devia(const STensor& spBatchOutDevia, SNnDevia& spWeightDevia) = 0;

        //
        // 更新权重
        //
        virtual int update(const SNnDevia& spWeightDevia) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnCompiledUnit)

#endif//__SimpleWork_NN_SNnCompiledUnit_H__
