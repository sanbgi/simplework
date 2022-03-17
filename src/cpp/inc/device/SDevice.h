#ifndef __SimpleWork_Device_SDevice_h__
#define __SimpleWork_Device_SDevice_h__

#include "device.h"
#include "PKernelVariable.h"
#include "SDeviceFactory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SKernelMemory;

//
// 计算设备
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Device)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IDevice", 220307)

        //
        // 创建设备内存
        //
        virtual int createKernelMemory(SKernelMemory& spKernelMemory, int nSize, void* pData = nullptr) = 0;

        //
        // 创建设备内存
        //
        virtual int createKernelMemory(SKernelMemory& spKernelMemory, const SKernelMemory& spMemory) = 0;

        //
        // 执行运算任务
        //
        virtual int runKernel(
                        const PRuntimeKey& kernelKey, 
                        int nArgs, 
                        PKernelVariable pArgs[], 
                        int nRanges = 0, 
                        int pRanges[]=nullptr) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SDevice getDevice(const char* szDeviceName) {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getDevice(szDeviceName, spDevice);
        return spDevice;
    }

    static SDevice defaultKernelDevice() {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getDefaultKernelDevic(spDevice);
        return spDevice;
    }

    static SDevice defaultHostDevice() {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getDefaultHostDevic(spDevice);
        return spDevice;
    }

    static const SDevice& cpu() {
        static SDevice s_spDevice = getDevice("cpu");
        return s_spDevice;
    }
 
    static const SDevice& cuda() {
        static SDevice s_spDevice = getDevice("cuda");
        return s_spDevice;
    }

    static const SDevice& opencl() {
        static SDevice s_spDevice = getDevice("opencl");
        return s_spDevice;
    }

    bool isCpu(){
        return getPtr() == cpu().getPtr();
    }

    bool isCuda() {
        return getPtr() == cuda().getPtr();
    }

    bool isOpencl() {
        return getPtr() == opencl().getPtr();
    }

public://常用辅助函数
    //
    // 内存初始化为零
    //
    int memoryZero(void* pDevicePointer, int iOffset, int nBytes) const {
        static PRuntimeKey sKernelKey("sw.device.MemoryZero.ucharEval");
        PKernelVariable pArgs[] = {
            {pDevicePointer},
            {iOffset}
        };
        return getPtr()->runKernel(sKernelKey, 2, pArgs, 1, &nBytes);
    }

    //
    // 内存片段拷贝，从pSrc(iSrcOffset) --> pDest(iDestOffset）
    //
    int memoryCopy(void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nBytes) const {
        static PRuntimeKey sKernelKey("sw.device.MemoryCopy.ucharEval");
        PKernelVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        return getPtr()->runKernel(sKernelKey, 4, pArgs, 1, &nBytes);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Device)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SDevice_h__