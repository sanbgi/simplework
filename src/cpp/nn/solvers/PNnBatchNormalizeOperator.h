#ifndef __SimpleWork_NN_PNnBatchNormalizeOperator_H__
#define __SimpleWork_NN_PNnBatchNormalizeOperator_H__

#include "operator.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 批量标准化
//
struct PNnBatchNormalizeOperator {
    SIMPLEWORK_PDATAKEY(PNnBatchNormalizeOperator, "sw.nn.NnBatchNormalizeOperator")

    //
    // 避免除零的偏置，一般为一个很小的值
    //
    double dEsp;

    //
    // 最小标准化批数据量
    //
    int nMinBatchs;

    //
    // 平均值
    //
    STensor* pAvg;

    //
    // 方差值
    //
    STensor* pVariance;
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnBatchNormalizeOperator_H__