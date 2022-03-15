#ifndef __SimpleWork_Device_Memory_h__
#define __SimpleWork_Device_Memory_h__

#include "device.h"
#include "SDevice.h"
#include "PDeviceMemory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 设备内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(DeviceMemory, "sw.device.DeviceMemory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDeviceMemory", 220307)

        //
        // 获取内存大小
        //
        virtual int getSize() = 0;

        //
        // 获取内存值
        //
        virtual void* getData(const SDevice& spDevice) = 0;

        //
        // 获取设备
        //
        virtual int getDevice(SDevice& spDevice) = 0;

        //
        // 修改内存值
        //
        virtual int writeMemory(const PMemory& cpuMemory, int iOffset=0) = 0;

        //
        // 读取内存值
        //
        virtual int readMemory(const PMemory& cpuMemory, int iOffset=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    int size() const {
        IFace* pFace = getPtr();
        return pFace != nullptr ? pFace->getSize() : 0;
    }

    void* data(const SDevice& spDevice = SDevice::cpu()) const {
        IFace* pFace = getPtr();
        return (pFace != nullptr) ? pFace->getData(spDevice) : nullptr;
    }

    SDevice device() const {
        SDevice spDevice;
        IFace* pFace = getPtr();
        if(pFace) pFace->getDevice(spDevice);
        return spDevice;
    }

    static SDeviceMemory createDeviceMemory(const SDevice& spDevice, int nSize, void* pData=nullptr) {
        return SObject::createObject(SDeviceMemory::__getClassKey(), CData<PDeviceMemory>({spDevice.getPtr(),nSize,pData}));
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(DeviceMemory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_Memory_h__