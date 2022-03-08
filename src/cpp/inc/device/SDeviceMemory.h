#ifndef __SimpleWork_Core_DeviceMemory_h__
#define __SimpleWork_Core_DeviceMemory_h__

#include "device.h"
#include "SDevice.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER0(DeviceMemory)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDeviceMemory", 220307)

        //
        // 获取设备
        //
        virtual int getDevice(SDevice& spDevice) = 0;

        //
        //获取内存
        //
        virtual int getMemory(PMemory& devcieMemory) = 0;

        //
        // 修改内存值
        //
        virtual int setDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0) = 0;

        //
        // 读取内存值(阻塞执行)
        //
        virtual int getDeviceMemory(PMemory cpuDeviceMemory, int iOffset=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SDevice device() const{
        SDevice spDevice;
        IFace* pFace = getPtr();
        if(pFace) pFace->getDevice(spDevice);
        return spDevice;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(DeviceMemory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Core_DeviceMemory_h__