#ifndef __SimpleWork_Device_SKernalOperator_h__
#define __SimpleWork_Device_SKernalOperator_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

struct PKernalCtx {
    int nRanges;
    int* pRanges;
    int* pRangeSizes;
};
typedef void (*FKernalFunc)(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]);

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER0(KernalOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IKernalOperator", 220307)

        //
        // 获取内核函数地址
        //
        virtual FKernalFunc getKernalFunc(const char* szName) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(KernalOperator)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SKernalOperator_h__