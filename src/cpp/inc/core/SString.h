#ifndef __SimpleWork_Core_SString_h__
#define __SimpleWork_Core_SString_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
//
//  管道，推入管道数据，经过管道处理后，返回结果数据，适用于立即处理并且立即获取返回结果的管道模式
//
//
SIMPLEWORK_INTERFACECLASS_ENTER0(String)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IString", 011230)

        //
        // 推入数据，并且通过pReceiver获得返回结果
        //
        virtual const char* getCharPtr() = 0; 

    SIMPLEWORK_INTERFACE_LEAVE

    SIMPLEWORK_INTERFACECLASS_ENTER(StringFactory, "sw.core.StringFactory")
        SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.IStringFactory", 211230)
            //
            //  创建字符串
            //
            virtual int createString(const char* sz, SString& spString) = 0;
        SIMPLEWORK_INTERFACE_LEAVE
    SIMPLEWORK_INTERFACECLASS_LEAVE(StringFactory)

public:
    SString(const char* sz) {
        getFactory()->createString(sz, (*this));
    }

public:
    operator const char*() {
        if( *this ) {
            return (*this)->getCharPtr();
        }
        return nullptr;
    }

private:
    static SStringFactory& getFactory() {
        static SStringFactory g_factory = SObject::createObject<SStringFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(String)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SString_h__