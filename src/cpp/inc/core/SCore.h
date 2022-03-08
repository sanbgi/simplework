#ifndef __SimpleWork_Core_SCore_h__
#define __SimpleWork_Core_SCore_h__

#include "core.h"

typedef unsigned int PID;
typedef unsigned int PDATATYPE;

__SimpleWork_Core_Namespace_Enter__

class SCtx;

//
// 字符串
//
SIMPLEWORK_INTERFACECLASS_ENTER(Core, "sw.core.Core")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ICore", 011230)
    
        //
        //  创建日志记录器
        //
        virtual int createCtx(const char* szModule, SCtx& spCtx) = 0;

        //
        //  根据字符串，获取字符串在运行时对应的唯一ID
        //
        virtual PID getRuntimeId(const char* szKey) = 0;


        //
        //  获取指定名字数据类型对应的数据类型ID
        //
        virtual PDATATYPE getDataType(const char* szDataTypeKey) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SCore& getFactory() {
        static SCore g_factory = SObject::createObject<SCore>();
        return g_factory;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Core)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_SCore_h__