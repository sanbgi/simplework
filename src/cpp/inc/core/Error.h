#ifndef __SimpleWork_Error_h__
#define __SimpleWork_Error_h__

__SimpleWork_Core_Namespace_Enter__

//
// 错误码定义
//
struct Error {
    enum ErrorType{
        ERRORTYPE_SUCCESS = 0,
        ERRORTYPE_FAILURE = 1,
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Error_h__