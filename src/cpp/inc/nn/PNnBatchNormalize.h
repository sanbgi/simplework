#ifndef __SimpleWork_NN_PNnBatchNormalize_H__
#define __SimpleWork_NN_PNnBatchNormalize_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 批量标准化
//
struct PNnBatchNormalize {
    SIMPLEWORK_PDATAKEY(PNnBatchNormalize, "sw.nn.NnBatchNormalize")

    //
    // 避免除零的偏置，一般为一个很小的值
    //
    double dEsp;

    //
    // 最小计算批量
    //
    int nMinBatch;

    //
    // 构造函数
    //
    PNnBatchNormalize(double esp=1.0e-8, int nMinBatch=10) {
        this->dEsp = esp;
        this->nMinBatch = nMinBatch;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnBatchNormalize_H__