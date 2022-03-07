#ifndef __SimpleWork_Parallel_PParallelMemory_h__
#define __SimpleWork_Parallel_PParallelMemory_h__

#include "parallel.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

struct PParallelMemory{
    int size;
    void* data;
};

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_PParallelMemory_h__