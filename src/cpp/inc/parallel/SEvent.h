#ifndef __SimpleWork_Parallel_Event_h__
#define __SimpleWork_Parallel_Event_h__

#include "parallel.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(Event, "sw.parallel.Event")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IEvent", 211202)

        virtual void wait() = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Event)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_Event_h__