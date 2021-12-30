#ifndef __SimpleWork_Core_SString_h__
#define __SimpleWork_Core_SString_h__

#include "core.h"
#include "SCoreFactory.h"

__SimpleWork_Core_Namespace_Enter__

//
// 字符串
//
SIMPLEWORK_INTERFACECLASS_ENTER0(String)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IString", 011230)

        //
        // 获取字符串指针
        //
        virtual const char* getCharPtr() = 0; 

    SIMPLEWORK_INTERFACE_LEAVE

public:
    SString(const char* sz) {
        SCoreFactory::getFactory()->createString(sz, (*this));
    }

public:
    operator const char*() {
        if( *this ) {
            return (*this)->getCharPtr();
        }
        return nullptr;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(String)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SString_h__