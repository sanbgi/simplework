#ifndef __SimpleWork_Object_h__
#define __SimpleWork_Object_h__

#include "core.h"
#include "IObject.h"
#include "SModule.h"

__SimpleWork_Core_Namespace_Enter__

//
// 对象接口基类，注意：
//
//  1, 所有对象接口都需要直接或间接从这个接口派生
//  2, IObject的接口函数都不允许直接访问，避免引用计数错误，或者非法指针访问，请适用智能指针
//
SIMPLEWORK_INTERFACECLASS_ENTER(Object, "sw.core.Object")

public:
    typedef IObject IFace;

    template<typename TObject> static inline SObject createObject(const PArg* pData = nullptr) {
        SObject spObject;
        createObject(TObject::__getClassKey(), pData, spObject);
        return spObject;
    }

    static SObject createObject(const char* szClassKey, const PArg* pData = nullptr) {
        SObject spObject;
        createObject(szClassKey, pData, spObject);
        return spObject;
    }

    static int createObject(const char* szClassKey, const PArg* pData, SObject& rObject) {
        return SModule::getSimpleWork()->createObject(szClassKey, pData, rObject);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Object)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Object_h__