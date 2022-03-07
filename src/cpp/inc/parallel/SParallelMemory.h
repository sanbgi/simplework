#ifndef __SimpleWork_Parallel_ParallelMemory_h__
#define __SimpleWork_Parallel_ParallelMemory_h__

#include "parallel.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(ParallelMemory, "sw.parallel.ParallelMemory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IParallelMemory", 220307)

        //
        // 获取内存值
        //
        //virtual int getMemory(IVisitor<PParallelMemory> cb) = 0;

        //
        // 修改内存值
        //
        //virtual int setMemory(PParallelMemory block, int offset=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(ParallelMemory)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_ParallelMemory_h__