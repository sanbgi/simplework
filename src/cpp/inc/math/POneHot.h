#ifndef __SimpleWork_MATH_POneHot_H__
#define __SimpleWork_MATH_POneHot_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER 

struct POneHot {
    SIMPLEWORK_PDATAKEY(POneHot, "sw.math.OneHot")

    int nClassify;
    PDATATYPE idType;
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_MATH_POneHot_H__