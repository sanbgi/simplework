#ifndef __SimpleWork_IFactory__h__
#define __SimpleWork_IFactory__h__

#include "core.h"
#include "SObject.h"

__SimpleWork_Core_Namespace_Enter__

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Factory)

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.IFactory", 211202)
        virtual int createObject(SObject& rObject) const = 0;
    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(Factory)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IFactory__h__