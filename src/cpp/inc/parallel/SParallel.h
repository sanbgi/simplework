#ifndef __SimpleWork_Parallel_Parallel_h__
#define __SimpleWork_Parallel_Parallel_h__

#include "parallel.h"
class SParallelMemory;
class SParallelOperator;

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER(Parallel, "sw.parallel.Parallel")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IParallel", 220307)

        

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Parallel)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_Parallel_h__