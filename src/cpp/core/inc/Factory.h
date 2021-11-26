#ifndef __SimpleWork_Factory_h__
#define __SimpleWork_Factory_h__

namespace SimpleWork {

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

}//namespace SimpleWork

#endif//__SimpleWork_Factory_h__