#ifndef __SimpleWork_NN_PNnConv_H__
#define __SimpleWork_NN_PNnConv_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

//
// 卷积核参数定义
//
//  注意，这里和行业里面的卷积有点不一样，其差别在于
//      行业卷积核: nConvs(输出特征层数) * nHeight * nWidth * nDepth
//      此处卷积核：nLayers(输出特征层数) * nShiftConvs * nHeight * nWidth * nDepth
//  当nShiftConvs为1时，退化为行业标准卷积核
//
struct PNnConv {
    SIMPLEWORK_PDATAKEY(PNnConv, "sw.nn.NnConv")

    //
    // 卷积核宽度
    //
    int nWidth;
    //
    // 卷积核高度
    //
    int nHeight;
    //
    // 卷积核层数，对应着输出的特征平面数
    //
    int nLayers;
    //
    // 每层有多少个卷积核循环使用（默认为1，则与普通行业卷积相同）
    //
    int nShiftConvs;
    //
    // 填充模式，"valid" 或者 "same"
    //
    const char *szPadding;
    //
    // 激活函数，"tanh" "sigmod" "relu"等等
    //
    const char *szActivator;

    //
    // 构造函数
    //
    PNnConv(int nConvWidth, int nConvHeight, int nConvLayers, int nShiftConvs = 1, const char* szPadding = nullptr, const char* szActivator = nullptr) {
        this->nWidth = nConvWidth;
        this->nHeight = nConvHeight;
        this->nLayers = nConvLayers;
        this->nShiftConvs = nShiftConvs;
        this->szActivator = szActivator;
        this->szPadding = szPadding;
    }
    PNnConv(){
        nShiftConvs = 1;
        szActivator = nullptr;
        szPadding = nullptr;
    }
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnConv_H__