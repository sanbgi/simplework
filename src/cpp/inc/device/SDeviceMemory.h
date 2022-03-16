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
        // 获取真正的内核内存(如果是内核内存，则直接返回自己)
        //
        virtual int getKernelMemory(SDeviceMemory& spKernelMemory) = 0;

        //
        // 转换为目标设备内存
        //
        virtual int toDevice(const SDevice& spDevice, SDeviceMemory& spMemory) = 0;

        //
        // 写回内存
        //
        virtual int writeMemory(const SDeviceMemory& spMemory) = 0;

        //
        // 修改内存值
        //
        virtual int writeMemory(int nSize, void* pData, int iOffset=0) = 0;

        //
        // 读取内存值
        //
        virtual int readMemory(int nSize, void* pData, int iOffset=0) = 0;

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

    SDeviceMemory toDevice(const SDevice& spDevice) {
        SDeviceMemory toMemory;
        IFace* pFace = getPtr();
        if(pFace) pFace->toDevice(spDevice, toMemory);
        return toMemory;
    }

    static SDeviceMemory createDeviceMemory(const SDevice& spDevice, int nSize, void* pData=nullptr) {
        return SObject::createObject(SDeviceMemory::__getClassKey(), CData<PDeviceMemory>({spDevice,nSize,pData}));
    }

    static SDeviceMemory createDeviceMemory(const SDevice& spDevice, const SDeviceMemory& spKernelMemory) {
        return SObject::createObject(SDeviceMemory::__getClassKey(), CData<PDeviceMemory>({spDevice, spKernelMemory}));
    }

 
SIMPLEWORK_INTERFACECLASS_LEAVE(DeviceMemory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_Memory_h__