#ifndef __SimpleWork_Error_h__
#define __SimpleWork_Error_h__

__SimpleWork_Core_Namespace_Enter__

//
// 错误码定义
//
struct Error {
    enum{
        Success = 0,
        Failure = 1,
    };
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Error_h__