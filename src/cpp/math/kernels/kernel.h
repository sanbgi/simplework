#ifndef __SimpleWork_Math_Operators_Operator_H__
#define __SimpleWork_Math_Operators_Operator_H__

#include "../math.h"
#include <string>

class CKernelArgs {
public:
    template<typename Q> static Q& getArg(PMemory &sMemory) {
        return *(Q*)sMemory.data;
    }
};
#define _KArg(x) CKernelArgs::getArg<x>
#define global
#define __global
#define volatile
#define kernel

#endif//__SimpleWork_Math_Operators_Operator_H__