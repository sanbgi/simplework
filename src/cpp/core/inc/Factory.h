#ifndef __SimpleWork_Factory_h__
#define __SimpleWork_Factory_h__

__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_CLASS_ENTER(Factory)

public:
    Factory(const char* szClassKey) {
        *this = getSimpleWorkModule()->createFactory(szClassKey);
    }

public:
    Object createObject() {
        return !isNullptr() ? getPtr()->createObject() : Object();
    }

SIMPLEWORK_CLASS_LEAVE

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Factory_h__