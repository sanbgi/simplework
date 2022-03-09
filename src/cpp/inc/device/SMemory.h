#ifndef __SimpleWork_Device_Memory_h__
#define __SimpleWork_Device_Memory_h__

#include "device.h"
#include "SDevice.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER


//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(Memory, "sw.device.Memory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IMemory", 220307)
        //
        // 获取设备
        //
        virtual int getDevice(SDevice& spDevice) = 0;

        //
        // 获取内存值
        //
        virtual int getMemoryInDevice(const SDevice& spDevice, PMemory& deviceMemory) = 0;

        //
        // 修改内存值
        //
        virtual int setMemory(PMemory cpuMemory, int iOffset=0) = 0;

        //
        // 读取内存值
        //
        virtual int getMemory(PMemory cpuMemory, int iOffset=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    SDevice device() const {
        SDevice spDevice;
        IFace* pFace = getPtr();
        if(pFace) pFace->getDevice(spDevice);
        return spDevice;
    }

    static SMemory createMemory(const PMemory& sMemory) {
        return SObject::createObject(SMemory::__getClassKey(), CData<PMemory>(sMemory));
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Memory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_Memory_h__