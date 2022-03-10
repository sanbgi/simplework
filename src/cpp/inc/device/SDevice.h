#ifndef __SimpleWork_Device_SDevice_h__
#define __SimpleWork_Device_SDevice_h__

#include "device.h"
#include "SDeviceFactory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SDeviceEvent;
class SMemory;

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Device)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDevice", 220307)

        //
        // 创建设备内存
        //
        virtual int createMemory(const PMemory& cpuMemory, SMemory& spDeviceMemory) = 0;

        //
        // 创建设备内存
        //
        virtual int createMemory(const SMemory& spSrcMemory, SMemory& spDeviceMemory) = 0;

        //
        // 执行运算任务
        //
        virtual int runKernel(
                        const PRuntimeKey& opKey, 
                        int nArgs, 
                        PMemory pArgs[], 
                        int nRanges = 0, 
                        int pRanges[]=nullptr, 
                        SDeviceEvent* pEvent=nullptr) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SDevice defaultDevice() {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getDefaultDevice(spDevice);
        return spDevice;
    }

    static SDevice cpu() {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getCpuDevice(spDevice);
        return spDevice;
    }
 
SIMPLEWORK_INTERFACECLASS_LEAVE(Device)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SDevice_h__