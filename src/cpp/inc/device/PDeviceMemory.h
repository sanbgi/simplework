#ifndef __SimpleWork_Device_PDeviceMemory_h__
#define __SimpleWork_Device_PDeviceMemory_h__

#include "device.h"
#include "SDevice.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

struct PDeviceMemory{
    SIMPLEWORK_PDATAKEY(PDeviceMemory, "sw.core.DeviceMemory")

    // 设备
    IDevice* pDevice;

    // 大小
    int size;

    // 指针
    void* data;

    PDeviceMemory(IDevice* pDevice, int size, void* data=nullptr) {
        this->pDevice = pDevice;
        this->size = size;
        this->data = data;
    }
};


SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_PDeviceMemory_h__