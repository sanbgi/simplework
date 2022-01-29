#ifndef __SimpleWork_NN_PNnLinear_H__
#define __SimpleWork_NN_PNnLinear_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 线性变换
//
struct PNnLinear {
    SIMPLEWORK_PDATAKEY(PNnLinear, "sw.nn.NnLinear")

    //
    // 神经元数
    //
    int nCells;

    //
    // 是否需要偏置
    //
    bool bBais;

    //
    // 激活函数
    //
    const char* szActivator;

    //
    // 构造函数
    //
    PNnLinear(int nCells, bool bBais = false, const char* szActivator = nullptr) {
        this->nCells = nCells;
        this->bBais = bBais;
        this->szActivator = szActivator;
    }
    PNnLinear(){
        szActivator = nullptr;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnLinear_H__