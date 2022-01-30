#ifndef __SimpleWork_NN_PNnWeight_H__
#define __SimpleWork_NN_PNnWeight_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 权重参数定义
//
struct PNnWeight {
    SIMPLEWORK_PDATAKEY(PNnWeight, "sw.nn.NnWeight")

    //
    // 权重平均值
    //
    float dAvg;

    //
    // 权重维度
    //
    SDimension spDim;

    //
    // 构造函数
    //
    PNnWeight(const SDimension& spDim, float dAvg) {
        this->spDim = spDim;
        this->dAvg = dAvg;
    }
    PNnWeight(){
        dAvg = 0;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnWeight_H__