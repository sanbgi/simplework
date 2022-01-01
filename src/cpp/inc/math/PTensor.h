#ifndef __SimpleWork_MATH_PTensor_H__
#define __SimpleWork_MATH_PTensor_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER 

struct PTensor {
    SIMPLEWORK_PDATAKEY(PTensor, "sw.math.Tensor")

    //
    //  张量数据类型
    //      SData::getBasicTypeIdentifier<int>      int类型张量
    //      SData::getBasicTypeIdentifier<float>    float类型张量
    //
    unsigned int idType;

    //
    //  张量的维度数量
    //
    int nDims;

    //
    //  张量的每一个维度尺寸
    //
    int* pDimSizes;

    //
    //  数据个数，与所有维度尺寸的乘积相同
    //
    int nData;

    //
    //  张量数据地址
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