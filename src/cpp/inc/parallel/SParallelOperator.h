#ifndef __SimpleWork_Parallel_ParallelParallelOperator_h__
#define __SimpleWork_Parallel_ParallelOperator_h__

#include "parallel.h"
#include "SParallelMemory.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(ParallelOperator, "sw.parallel.ParallelOperator")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IParallelOperator", 220307)

        //
        // 并行计算
        //
        virtual int parallelize(int nRanges, int pRanges[], int nMemorys, SParallelMemory pMemorys[], int nArgs, PParallelMemory pArgs[] ) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(ParallelOperator)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_ParallelOperator_h__