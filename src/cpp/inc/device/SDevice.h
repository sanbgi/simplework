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
    // kernelid指针(临时存储，不能持久化此ID，程序重启以后，下一次未必相同)
    //
    int* pKernalId;
    //
    // kernel函数名字，比如：sw.math.TensorPlus.floatEval
    //
    const char* szKernalName;
};

//
// 内核计算变量
//
struct PKernalVariable{
    int size;
    union {
        unsigned char data[8];
        char c;
        int i;
        float f;
        double d;
        long long l;
        void* p;
    };

    inline PKernalVariable() {
        size = 0;
        l = 0;
    }

#define PKERNALVARIABLE_TYPE(type) \
    inline PKernalVariable(type v) {\
        size = sizeof(type);\
        *((type*)data) = v;\
    }

PKERNALVARIABLE_TYPE(char)
PKERNALVARIABLE_TYPE(unsigned char)
PKERNALVARIABLE_TYPE(short)
PKERNALVARIABLE_TYPE(unsigned short)
PKERNALVARIABLE_TYPE(int)
PKERNALVARIABLE_TYPE(unsigned int)
PKERNALVARIABLE_TYPE(float)
PKERNALVARIABLE_TYPE(double)
PKERNALVARIABLE_TYPE(long)
PKERNALVARIABLE_TYPE(long long)
PKERNALVARIABLE_TYPE(void*)
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
        virtual int createMemory(const SDeviceMemory& spMemory, SDeviceMemory& spDeviceMemory) = 0;

        //
        // 执行运算任务
        //
        virtual int runKernel(
                        const PKernalKey& kernelKey, 
                        int nArgs, 
                        PKernalVariable pArgs[], 
                        int nRanges = 0, 
                        int pRanges[]=nullptr) = 0;

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
 
     static SDevice opencl() {
        SDevice spDevice;
        SDeviceFactory::getFactory()->getOpenclDevice(spDevice);
        return spDevice;
    }

public://常用辅助函数
    //
    // 内存初始化为零
    //
    int memoryZero(void* pDevicePointer, int iOffset, int nBytes) const {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {
            {pDevicePointer},
            {iOffset}
        };
        return getPtr()->runKernel({&sSetKernalId, "sw.device.MemoryZero.ucharEval"}, 2, pArgs, 1, &nBytes);
    }

    //
    // 内存片段拷贝，从pSrc(iSrcOffset) --> pDest(iDestOffset）
    //
    int memoryCopy(void* pDest, int iDestOffset, void* pSrc, int iSrcOffset, int nBytes) const {
        static int sSetKernalId = 0;
        PKernalVariable pArgs[] = {
            {pDest},
            {iDestOffset},
            {pSrc},
            {iSrcOffset}
        };
        return getPtr()->runKernel({&sSetKernalId, "sw.device.MemoryCopy.ucharEval"}, 4, pArgs, 1, &nBytes);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Device)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SDevice_h__