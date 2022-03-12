#ifndef __SimpleWork_Math_Operators_Operator_H__
#define __SimpleWork_Math_Operators_Operator_H__

#include "../nn.h"
#include <math.h>
#include <string>

using namespace sw;

#define _KArg(tp,id) *(tp*)pArgs[id].data
#define global
#define __global
#define volatile
#define kernel
static int atomic_cmpxchg(unsigned int* pV, int oldV, int newV) {
    *pV = newV;
    return oldV;
}

static int get_global_offset(int) {
    return 0;
}

#endif//__SimpleWork_Math_Operators_Operator_H__