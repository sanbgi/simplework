#ifndef __SimpleWork_Device_SDevice_h__
#define __SimpleWork_Device_SDevice_h__

#include "device.h"
#include "SDeviceFactory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SDeviceEvent;
class SDeviceMemory;

//
// 内核标识
//
struct PKernalKey {
    //
    // kernelid指针
    //
    int* pKernalId;
    // 
    // kernel程序名字，一般为: sw.math.TensorPlus等
    //
    const char* szProgramName;
    //
    // kernel函数名字，比如：floatEval
    //
    const char* szKernalName;
};

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Device)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDevice", 220307)

        //
        // 创建设备内存
        //
        virtual int createMemory(const PMemory& cpuMemory, SDeviceMemory& spDeviceMemory) = 0;

        //
        // 创建设备内存
        //
        virtual int createMemory(const SDeviceMemory& spSrcMemory, SDeviceMemory& spDeviceMemory) = 0;

        //
        // 执行运算任务
        //
        virtual int runKernel(
                        const PKernalKey& kernelKey, 
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