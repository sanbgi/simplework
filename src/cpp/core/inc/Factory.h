#ifndef __SimpleWork_IFactory__h__
#define __SimpleWork_IFactory__h__

__SimpleWork_Core_Namespace_Enter__

//
// 对象工场对象的接口
//
class Factory {
    SIMPLEWORK_OBJECT_INTERFACE_ENTER(Factory, IObject, "sw.core.IFactory", 211202)
        virtual Object createObject() = 0;
    SIMPLEWORK_OBJECT_INTERFACE_LEAVE(Factory)
};
typedef Factory::IFace IFactory;

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IFactory__h__