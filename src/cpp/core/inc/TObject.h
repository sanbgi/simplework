#ifndef __SimpleWork_Object_h__
#define __SimpleWork_Object_h__
__SimpleWork_Core_Namespace_Enter__

template<typename TInterface=IObject> struct Object {

public:
    template<typename Q> Object& operator=(const Object<Q>& src) {
        _spObject = src._spObject;
        return *this;
    }
    template<typename Q> Object& operator=(const SmartPtr<Q>& src) {
        _spObject = src;
        return *this;
    }
    template<typename Q> Object& operator=(Q* pSrc) {
        _spObject = pSrc;
        return *this;
    }

public:
    SmartPtr<TInterface> _spObject;
};

__SimpleWork_Core_Namespace_Leave__
#endif//__SimpleWork_Object_h__