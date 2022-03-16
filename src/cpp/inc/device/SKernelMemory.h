#ifndef __SimpleWork_Device_KernelMemory_h__
#define __SimpleWork_Device_KernelMemory_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 设备内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(KernelMemory, "sw.device.KernelMemory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IKernelMemory", 220307)

        //
        // 获取内存大小
        //
        virtual int getSize() = 0;

        //
        // 获取内存值
        //
        virtual void* getData() = 0;

        //
        // 获取设备
        //
        virtual int getDevice(SDevice& spDevice) = 0;

        //
        // 写回内存
        //
        virtual int writeMemory(const SKernelMemory& spMemory) = 0;

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

    void* data() const {
        IFace* pFace = getPtr();
        return (pFace != nullptr) ? pFace->getData() : nullptr;
    }

    SDevice device() const {
        SDevice spDevice;
        IFace* pFace = getPtr();
        if(pFace) pFace->getDevice(spDevice);
        return spDevice;
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(KernelMemory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_KernelMemory_h__