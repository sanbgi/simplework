#ifndef __SimpleWork_MATH_PVector_H__
#define __SimpleWork_MATH_PVector_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER 

struct PVector {
    SIMPLEWORK_PDATAKEY(PVector, "sw.math.Vector")

    //
    //  数据个数
    //
    int size;

    //
    //  张量数据地址
    //
    union {
        //
        //  数据地址
        //
        void* data;
        int* pIntArray;
        float* pFloatArray;
        double* pDoubleArray;
        unsigned char* pByteArray;
    };
};

struct PDeviaVector {
    //
    // 大小
    //
    int size;

    //
    // 数值数组
    //
    void* data;

    //
    // 偏导数组
    //
    void* devia;
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE


#endif//__SimpleWork_MATH_PVector_H__