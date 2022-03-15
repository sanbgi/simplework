#ifndef __SimpleWork_Device_PDeviceMemory_h__
#define __SimpleWork_Device_PDeviceMemory_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SDevice;
class SDeviceMemroy;

struct PDeviceMemory{
    SIMPLEWORK_PDATAKEY(PDeviceMemory, "sw.core.DeviceMemory")

    // 设备
    const SDevice* pDevice;

    //
    // 内核内存
    //
    const SDeviceMemory* pKernelMemory;

    // 大小
    int size;

    // 指针
    void* data;

    PDeviceMemory(const SDevice& spDevice, int size, void* data=nullptr) {
        this->pDevice = &spDevice;
        this->pKernelMemory = nullptr;
        this->size = size;
        this->data = data;
    }

    PDeviceMemory(const SDevice& spDevice, const SDeviceMemory& spKernelMemory) {
        this->pDevice = &spDevice;
        this->pKernelMemory = &spKernelMemory;
    }
};


SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_PDeviceMemory_h__