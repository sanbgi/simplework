#ifndef __SimpleWork_NN_PNnPool_H__
#define __SimpleWork_NN_PNnPool_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 池化层参数
//
struct PNnPool {
    SIMPLEWORK_PDATAKEY(PNnPool, "sw.nn.NnPool")

    //
    // 池化宽度
    //
    int nWidth;
    //
    // 池化高度
    //
    int nHeight;
    //
    // 步长宽度
    //
    int nStrideWidth;
    //
    // 步长高度
    //
    int nStrideHeight;
    //
    // 填充模式，"valid" 或 "same"
    //
    const char * szPadding;

    //
    // 构造函数
    //
    PNnPool(int nPoolWidth, int nPoolHeight, int nStrideWidth, int nStrideHeight, const char* szPadding = nullptr) {
        this->nWidth = nPoolWidth;
        this->nHeight = nPoolHeight;
        this->nStrideWidth = nStrideWidth;
        this->nStrideHeight = nStrideHeight;
        this->szPadding = szPadding;
    }
    PNnPool(){
        szPadding = nullptr;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnPool_H__