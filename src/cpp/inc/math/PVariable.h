#ifndef __SimpleWork_MATH_PVariable_H__
#define __SimpleWork_MATH_PVariable_H__

#include "math.h"

SIMPLEWORK_MATH_NAMESPACE_ENTER 

struct PVariable {

    SIMPLEWORK_PDATAKEY(PVariable, "sw.math.Variable")

    enum EVariable {
        VUNKNOWN,
        VBOOL,
        VCHAR,
        VSHORT,
        VINT,
        VLONG,
        VUCHAR,
        VUINT,
        VPOINTER
    };

    //
    //  变量数据类型
    //
    EVariable eType;

    //
    //  张量数据地址
    //
    union {
        //
        //  数据地址
        //
        bool vBool;
        char vChar;
        short vShort;
        int vInt;
        long vLong;
        unsigned char vUChar;
        unsigned int vUInt;
        float vFloat;
        double vDouble;
        void* vPointer;
    };
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE


#endif//__SimpleWork_MATH_PVariable_H__