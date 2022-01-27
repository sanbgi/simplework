#ifndef __SimpleWork_NN_PNnPool_H__
#define __SimpleWork_NN_PNnPool_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

struct PNnPool {
    SIMPLEWORK_PDATAKEY(PNnPool, "sw.nn.NnPool")

    int nWidth;
    int nHeight;
    int nStrideWidth;
    int nStrideHeight;
    const char * szPadding;
};

SIMPLEWORK_NN_NAMESPACE_LEAVE


#endif//__SimpleWork_NN_PNnPool_H__