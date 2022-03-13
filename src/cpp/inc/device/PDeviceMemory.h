#ifndef __SimpleWork_Device_PDeviceMemory_h__
#define __SimpleWork_Device_PDeviceMemory_h__

#include "device.h"
#include "SDevice.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER


struct PDeviceMemory{
    SIMPLEWORK_PDATAKEY(PDeviceMemory, "sw.core.DeviceMemory")

    //
    // 设备
    //
    SDevice spDevice;

    //
    // 初始化内存
    //
    PMemory cpuMemory;
};


SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_PDeviceMemory_h__