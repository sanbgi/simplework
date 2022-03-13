#ifndef __SimpleWork_NN_SNnExtraTensor_H__
#define __SimpleWork_NN_SNnExtraTensor_H__

#include "nn.h"

using namespace sw;

struct PNnExtraTensor {
    SIMPLEWORK_PDATAKEY(PNnExtraTensor, "sw.nn.NnExtraTensor")

    //
    // 原始数据
    //
    STensor spTensor;

    //
    // 扩展存储数组
    //
    int nExtras;
    SObject* pExtras; 
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnExtraTensor)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnExtraTensor", 220210)

        //
        // 获得基础数据
        //
        virtual int getResizeData(PNnExtraTensor& rResizeTensor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnExtraTensor)


#endif//__SimpleWork_NN_SNnExtraTensor_H__
