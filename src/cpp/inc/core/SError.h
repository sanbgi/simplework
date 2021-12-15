#ifndef __SimpleWork_SError_h__
#define __SimpleWork_SError_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// 错误码定义
//
struct SError {
    enum ErrorType{
        ERRORTYPE_SUCCESS = 0,
        ERRORTYPE_FAILURE = 1,
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_SError_h__