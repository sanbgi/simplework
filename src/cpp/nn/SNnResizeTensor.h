#ifndef __SimpleWork_NN_SNnResizeTensor_H__
#define __SimpleWork_NN_SNnResizeTensor_H__

#include "nn.h"

using namespace sw;

struct PNnResizeTensor {
    SIMPLEWORK_PDATAKEY(PNnResizeTensor, "sw.nn.NnResizeTensor")

    //
    // 原始数据
    //
    STensor spSrc;

    //
    // 新数据维度信息
    //
    SDimension spResizeDimension;

    //
    // 新数据相对于原始数据的起始位置
    //
    int iResizeOffset;

    //
    // 额外存储参数
    //
    SObject spExtra;
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
