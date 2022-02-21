#ifndef __SimpleWork_NN_PNnState_H__
#define __SimpleWork_NN_PNnState_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 权重参数定义
//
struct PNnState {
    SIMPLEWORK_PDATAKEY(PNnState, "sw.nn.NnState")

    //
    // 权重维度
    //
    SDimension spDim;

    //
    // 构造函数
    //
    PNnState(const SDimension& spDim) {
        this->spDim = spDim;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnState_H__