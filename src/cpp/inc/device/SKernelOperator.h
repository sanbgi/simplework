#ifndef __SimpleWork_Device_SKernelOperator_h__
#define __SimpleWork_Device_SKernelOperator_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

struct PKernelCtx {
    int nRanges;
    int* pRanges;
    int* pRangeSizes;
};
typedef void (*FKernelFunc)(const PKernelCtx* pCtx, int nArgs, PKernelVariable pArgs[]);

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER0(KernelOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IKernelOperator", 220307)

        //
        // 获取内核函数地址
        //
        virtual FKernelFunc getKernelFunc(const char* szName) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(KernelOperator)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SKernelOperator_h__