#ifndef __SimpleWork_Device_DeviceEvent_h__
#define __SimpleWork_Device_DeviceEvent_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER0(DeviceEvent)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDeviceEvent", 220307)

        //
        // 阻塞直到执行结束
        //
        virtual void wait() = 0;

        //
        // 查询事件是否已经发生
        //
        virtual bool isSet() = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(DeviceEvent)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_DeviceEvent_h__