#ifndef __SimpleWork_Parallel_Device_h__
#define __SimpleWork_Parallel_Device_h__

#include "parallel.h"
#include "SMemory.h"

SIMPLEWORK_PARALLEL_NAMESPACE_ENTER

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER(Device, "sw.parallel.Device")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.parallel.IDevice", 211202)

        //
        // 创建内存
        //
        virtual int createMemory(PMemory data, SMemory& spMemory) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Device)

SIMPLEWORK_PARALLEL_NAMESPACE_LEAVE

#endif//__SimpleWork_Parallel_Device_h__