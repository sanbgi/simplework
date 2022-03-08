#ifndef __SimpleWork_Core_PMemory_h__
#define __SimpleWork_Core_PMemory_h__

#include "core.h"
#include "PData.h"

__SimpleWork_Core_Namespace_Enter__

struct PMemory{
    SIMPLEWORK_PDATAKEY(PMemory, "sw.core.Memory")

    //
    // 字节数
    //
    int size;

    //
    // 地址
    //
    union {
        void* data;
        int* pIntArray;
        float* pFloatArray;
        double* pDoubleArray;
        unsigned char* pByteArray;
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_PMemory_h__