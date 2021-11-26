#ifndef __SimpleWork_Error_h__
#define __SimpleWork_Error_h__

namespace SimpleWork {

//
// 错误码定义
//
struct Error {
    enum{
        Success = 0,
        Failure = 1,
    };
};

}//namespace SimpleWork

#endif//__SimpleWork_Error_h__