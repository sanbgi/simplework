#ifndef __SimpleWork_Factory__h__
#define __SimpleWork_Factory__h__

__SimpleWork_Core_Namespace_Enter__

class Factory : public TObject<Factory, IFactory>{
    SIMPLEWORK_OBJECT_DATACONVERSION(Factory)

public:
    Object createObject() {
        return Object::wrapPtr(getAutoPtr()->createObject());
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Factory__h__