#ifndef __SimpleWork_NN_SNnEvalOutTensor_H__
#define __SimpleWork_NN_SNnEvalOutTensor_H__

#include "nn.h"

using namespace sw;

struct PNnEvalOutTensor {
    SIMPLEWORK_PDATAKEY(PNnEvalOutTensor, "sw.nn.EvalOutTensor")

    STensor spBatchIn;
    STensor spBatchOp;
    SDimension spBatchOutDimension;
    int nBatchOutOffset;
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnEvalOutTensor)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnEvalOutTensor", 220209)

        //
        // 获取输入张量
        //
        virtual int getBatchIn(STensor& spBatchIn) = 0;

        //
        // 获取计算张量
        //
        virtual int getBatchOp(STensor& spBatchOp) = 0;


    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnEvalOutTensor)


#endif//__SimpleWork_NN_SNnEvalOutTensor_H__
