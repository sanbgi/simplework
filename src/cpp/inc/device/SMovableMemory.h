#ifndef __SimpleWork_Device_MovableMemory_h__
#define __SimpleWork_Device_MovableMemory_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER(MovableMemory, "sw.device.MovableMemory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IMovableMemory", 220307)

        //
        // 获取内存值
        //
        virtual int getDataInDevice(const SDevice& spDevice, PMemory* pDeviceMemory=nullptr) = 0;

        //
        // 修改内存值
        //
        virtual int setMemory(PMemory cpuMemory, int iOffset=0) = 0;

        //
        // 读取内存值
        //
        virtual int getMemory(PMemory cpuMemory, int iOffset=0) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SMovableMemory createMemory(const PMemory& sMemory) {
        return SObject::createObject(SMovableMemory::__getClassKey(), CData<PMemory>(sMemory));
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(MovableMemory)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_MovableMemory_h__