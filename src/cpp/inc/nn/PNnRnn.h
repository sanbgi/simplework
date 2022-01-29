#ifndef __SimpleWork_NN_PNnRnn_H__
#define __SimpleWork_NN_PNnRnn_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 池化层参数
//
struct PNnRnn {
    SIMPLEWORK_PDATAKEY(PNnRnn, "sw.nn.NnRnn")

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
    PNnRnn(int nCells, const char* szActivator = nullptr) {
        this->nCells = nCells;
        this->szActivator = szActivator;
    }
    PNnRnn(){
        szActivator = nullptr;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnRnn_H__