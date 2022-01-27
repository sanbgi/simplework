#ifndef __SimpleWork_NN_PNnConv_H__
#define __SimpleWork_NN_PNnConv_H__

#include "nn.h"

SIMPLEWORK_NN_NAMESPACE_ENTER 

struct PNnConv {
    SIMPLEWORK_PDATAKEY(PNnConv, "sw.nn.NnConv")

    int nWidth;
    int nHeight;
    int nLayers;
    int nShiftNnConvs;
    const char *szPadding;
    const char *szActivator;
};

SIMPLEWORK_NN_NAMESPACE_LEAVE

#endif//__SimpleWork_NN_PNnConv_H__