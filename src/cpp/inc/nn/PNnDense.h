#ifndef __SimpleWork_NN_PNnDense_H__
#define __SimpleWork_NN_PNnDense_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 池化层参数
//
struct PNnDense {
    SIMPLEWORK_PDATAKEY(PNnDense, "sw.nn.NnDense")

    //
    // 池化宽度
    //
    int nCells;

    //
    // 激活函数
    //
    const char* szActivator;

    //
    // 构造函数
    //
    PNnDense(int nCells, const char* szActivator = "relu") {
        this->nCells = nCells;
        this->szActivator = szActivator;
    }
    PNnDense(){
        szActivator = nullptr;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnDense_H__