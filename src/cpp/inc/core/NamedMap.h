#ifndef __SimpleWork_NamedMap__h__
#define __SimpleWork_NamedMap__h__

#include "Object.h"

__SimpleWork_Core_Namespace_Enter__

//
// 模块对象
//
SIMPLEWORK_INTERFACECLASS_ENTER(NamedMap, "sw.core.Map")
    
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.IMap", 211210)
    private:
        virtual NamedMap createMap() = 0;

    public:
        //
        // 获得指定名字的对象
        //
        virtual Object getAt(const char* szName) = 0;

        //
        // 设置指定名字的对象
        //
        virtual void putAt(const char* szName, Object& obj) = 0;

        friend class NamedMap;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    static inline NamedMap createMap() {
        return getFactory()->createMap();
    }

private:
    static NamedMap& getFactory() {
        static NamedMap g_factory = Object::createObject(NamedMap::getClassKey());
        return g_factory;
    }
SIMPLEWORK_INTERFACECLASS_LEAVE(NamedMap)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_NamedMap__h__