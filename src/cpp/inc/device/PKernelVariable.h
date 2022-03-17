#ifndef __SimpleWork_Device_PKernelVariable_h__
#define __SimpleWork_Device_PKernelVariable_h__

#include "device.h"
#include "SDeviceFactory.h"

SIMPLEWORK_DEVICE_NAMESPACE_ENTER

class SDeviceMemory;

//
// 内核计算变量
//
struct PKernelVariable{
    //
    //  数据类型: 值 / 内核指针 / 系统内存指针
    //
    enum{
        EValue,
        EKernelPointer,
        EDevicePointer
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
        SDeviceMemory* pDeviceMemory;
    };

    //
    // 针对内存数据的访问模式
    //  ReadOnly -- 内核只读取数据
    //  ReadWrite -- 内核不光是读取数据，而且会回写数据
    //  Write -- 内核只回写（全部）数据，并不读取
    //
    enum EMode{
        ENone,
        EReadOnly,
        EReadWrite,
        EWriteOnly,
    }mode;

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

    //
    // (性能不可接受，暂时不启用)
    //
    //PKernelVariable(const SDeviceMemory& spDeviceMemory, EMode eMode) {
    //    type = EKernelPointer;
    //    size = sizeof(void*);
    //    mode = eMode;
    //    pDeviceMemory = (SDeviceMemory*)&spDeviceMemory;
    //}

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