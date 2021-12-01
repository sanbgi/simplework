#ifndef __SimpleWork_Object__h__
#define __SimpleWork_Object__h__

__SimpleWork_Core_Namespace_Enter__

class Object : public TObject<Object, IObject>{
    SIMPLEWORK_OBJECT_DATACONVERSION(Object)

public:
    static Object createObject(const char* szClassKey) {
        return wrapPtr(getSimpleWork()->createObject(szClassKey));
    }
    static Object createFactory(const char* szClassKey) {
        return wrapPtr(getSimpleWork()->createFactory(szClassKey));
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Object__h__