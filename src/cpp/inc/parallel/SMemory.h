#ifndef __SimpleWork_Parallel_Memory_h__
#define __SimpleWork_Parallel_Memory_h__

#include "parallel.h"
#include "PMemory.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(Memory, "sw.parallel.Memory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IMemory", 211202)

        virtual void getMemory(IVisitor<PMemory,void> cb) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Memory)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_Memory_h__