#ifndef __SimpleWork_Core_SCoreFactory_h__
#define __SimpleWork_Core_SCoreFactory_h__

#include "core.h"

typedef unsigned int PID;

__SimpleWork_Core_Namespace_Enter__

class SString;
class SCtx;

//
// 字符串
//
SIMPLEWORK_INTERFACECLASS_ENTER(CoreFactory, "sw.core.CoreFactory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ICoreFactory", 011230)
    
        //
        //  创建日志记录器
        //
        virtual int createCtx(const char* szModule, SCtx& spCtx) = 0;

        //
        //  创建字符串
        //
        virtual int createString(const char* sz, SString& spString) = 0;

        //
        //  获取指定名字数据类型对应的数据类型ID
        //
        virtual PID getTypeIdentifier(const char* szDataTypeKey) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SCoreFactory& getFactory() {
        static SCoreFactory g_factory = SObject::createObject<SCoreFactory>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(CoreFactory)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SCoreFactory_h__