#ifndef __SimpleWork_NN_SNnResizeTensor_H__
#define __SimpleWork_NN_SNnResizeTensor_H__

#include "nn.h"

using namespace sw;

struct PNnResizeTensor {
    SIMPLEWORK_PDATAKEY(PNnResizeTensor, "sw.nn.NnResizeTensor")

    //
    // 原始数据
    //
    STensor spTensor;

    //
    // 额外存储参数
    //
    SObject spExtra1;
    SObject spExtra2;
};

//
// 神经网络计算器，为了实现高速计算，参数和函数地址，都是直接的指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(NnResizeTensor)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.nn.INnResizeTensor", 220210)

        //
        // 获得基础数据
        //
        virtual int getResizeData(PNnResizeTensor& rResizeTensor) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(NnResizeTensor)


#endif//__SimpleWork_NN_SNnResizeTensor_H__
