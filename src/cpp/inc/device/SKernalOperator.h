#ifndef __SimpleWork_Device_SKernalOperator_h__
#define __SimpleWork_Device_SKernalOperator_h__

#include "device.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

struct PKernalCtx {
    int nRanges;
    int* pRanges;
    int* pRangeSizes;
};

//
// 计算内存
//
SIMPLEWORK_INTERFACECLASS_ENTER0(KernalOperator)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.device.IKernalOperator", 220307)

        //
        // 运算
        //
        virtual int process(const PKernalCtx* pCtx, int nArgs, PMemory pArgs[]) = 0;

    SIMPLEWORK_INTERFACE_LEAVE
 
SIMPLEWORK_INTERFACECLASS_LEAVE(KernalOperator)

SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_SKernalOperator_h__