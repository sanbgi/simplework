#ifndef __SimpleWork_Core_PRuntimeKey_h__
#define __SimpleWork_Core_PRuntimeKey_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
//
//
struct PRuntimeKey{
    
    //
    // 运行时ID
    //
    PID runtimeId;

    //
    // 运行时KEY
    //
    const char* runtimeKey;

    //
    //
    //
    PRuntimeKey(const char* szKey=nullptr) {
        if(szKey != nullptr) {
            runtimeId = SCore::getFactory()->getRuntimeId(szKey);
            runtimeKey = szKey;
        }else{
            runtimeId = 0;
            runtimeKey = nullptr;
        }
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_PRuntimeKey_h__