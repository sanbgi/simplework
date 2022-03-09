#ifndef __SimpleWork_MATH_PTensor_H__
#define __SimpleWork_MATH_PTensor_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER 

//
// 备用类型，还未启用
//
struct PTensor {
    SIMPLEWORK_PDATAKEY(PTensor, "sw.math.Tensor")

    //
    // 维度信息
    //
    SDimension spDimension;

    //
    //  张量数据类型
    //      SData::getBasicTypeIdentifier<int>      int类型张量
    //      SData::getBasicTypeIdentifier<float>    float类型张量
    //
    PDATATYPE idType;

    //
    //  数据个数，与所有维度尺寸的乘积相同
    //
    int nData;

    //
    //  张量数据地址，务必注意，数据地址只有在特定的设备上从才能访问，比如：
    //
    union {
        //
        //  数据地址
        //
        void* pData;
        int* pIntArray;
        float* pFloatArray;
        double* pDoubleArray;
        unsigned char* pByteArray;
    };
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE


#endif//__SimpleWork_MATH_PTensor_H__