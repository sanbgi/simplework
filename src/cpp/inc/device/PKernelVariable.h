#ifndef __SimpleWork_Device_PKernelVariable_h__
#define __SimpleWork_Device_PKernelVariable_h__

#include "device.h"
#include "SDeviceFactory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

//
// 内核计算变量
//
struct PKernelVariable{
    //
    //  数据类型: 值 / 内核指针
    //
    enum{
        EValue,
        EKernelPointer,
    }type;

    //
    //  数据大小（值类型的数据大小，针对指针类型无效）
    //
    int size;

    union {
        unsigned char data[8];
        char c;
        int i;
        float f;
        double d;
        long long l;
        void* p;
    };

    inline PKernelVariable() {
        type = EValue;
        size = 0;
        l = 0;
    }

    PKernelVariable(void* pKernelPtr) {
        type = EKernelPointer;
        size = sizeof(void*);
        p = pKernelPtr;
    }

#define PKERNALVARIABLE_VALUE_TYPE(value_type) \
    inline PKernelVariable(value_type v) {\
        size = sizeof(value_type);\
        type = PKernelVariable::EValue;\
        *((value_type*)data) = v;\
    }

PKERNALVARIABLE_VALUE_TYPE(char)
PKERNALVARIABLE_VALUE_TYPE(unsigned char)
PKERNALVARIABLE_VALUE_TYPE(short)
PKERNALVARIABLE_VALUE_TYPE(unsigned short)
PKERNALVARIABLE_VALUE_TYPE(int)
PKERNALVARIABLE_VALUE_TYPE(unsigned int)
PKERNALVARIABLE_VALUE_TYPE(float)
PKERNALVARIABLE_VALUE_TYPE(double)
PKERNALVARIABLE_VALUE_TYPE(long)
PKERNALVARIABLE_VALUE_TYPE(long long)
};


SIMPLEWORK_DEVICE_NAMESPACE_LEAVE

#endif//__SimpleWork_Device_PKernelVariable_h__