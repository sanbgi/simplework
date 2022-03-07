#ifndef __SimpleWork_Parallel_ParallelEvent_h__
#define __SimpleWork_Parallel_ParallelEvent_h__

#include "parallel.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(ParallelEvent, "sw.parallel.ParallelEvent")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IParallelEvent", 220307)

        virtual void wait() = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(ParallelEvent)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_ParallelEvent_h__