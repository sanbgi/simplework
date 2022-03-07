#ifndef __SimpleWork_Parallel_ParallelDevice_h__
#define __SimpleWork_Parallel_ParallelDevice_h__

#include "parallel.h"
class SParallelMemory;
class SParallelOperator;

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER(ParallelDevice, "sw.parallel.ParallelDevice")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IParallelDevice", 220307)

        //
        // 创建内存
        //
        virtual int createMemory(PParallelMemory data, SParallelMemory& spMemory) = 0;

        //
        // 创建运算
        //
        virtual int createOperator(const char* szOperator, SParallelOperator& spOperator) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(ParallelDevice)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_ParallelDevice_h__