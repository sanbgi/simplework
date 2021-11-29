#ifndef __SimpleWork_AutoPrt__h__
#define __SimpleWork_AutoPrt__h__

__SimpleWork_Core_Namespace_Enter__

//
// 指针强制设置接口，由于设置的是强制转化后的地址，所以，接口不安全，也不能被外界使用
//
struct IPtrForceSaver {
    virtual int forceSetPtr(void* pPtr) = 0;
};

//
// 对象的智能指针定义，仅适用于从IObject派生的接口
//
template<typename TInterface> struct TAutoPtr {

public:
    TAutoPtr(){
        _ptr = nullptr;
    }
    TAutoPtr(const TAutoPtr& src) : _ptr(nullptr)  {
        initPtr(src._ptr);
    }
    TAutoPtr& operator=(const TAutoPtr& src) {
        assignPtr(src._ptr);
        return *this;
    }
    ~TAutoPtr() {
        releasePtr();
    }

    template<typename Q> TAutoPtr(Q* pPtr) : _ptr(nullptr) {
       initPtr(pPtr);
    }
    template<typename Q> TAutoPtr(const Q& src) : _ptr(nullptr) {
        initPtr(src.getPtr());
    }
    template<typename Q> TAutoPtr& operator=(Q* pSrc) {
        assignPtr(pSrc);
        return *this;
    }
    template<typename Q> TAutoPtr& operator=(const Q& src) {
        assignPtr(src.getPtr());
        return *this;
    }


public:
    TInterface* getPtr() const { 
        return _ptr; 
    }

public:
    bool isNullptr() const { 
        return _ptr == nullptr; 
    }
    TInterface* operator->() const {
        return _ptr; 
    }
    operator bool(){
        return _ptr != nullptr;
    }
    operator TInterface*() const {
        return _ptr;
    }

//
// 
//  私有成员及函数区域
// 
//
private:
    TInterface* _ptr = nullptr;

private:
    void initPtr(TInterface* pPtr = nullptr) {
        if( pPtr ) {
            _ptr = pPtr;
            _ptr->__swAddRef();
        }
    }
    template<typename Q> void initPtr(Q* pPtr = nullptr) {
        if( pPtr ) {
            struct CForceSetter : public IPtrForceSaver {
                CForceSetter(TAutoPtr* pAutoPtr) : _pPtr(pAutoPtr){}
                int forceSetPtr(void* pPtr) { return _pPtr->assignPtr((TInterface*)pPtr); }
                TAutoPtr* _pPtr;
            }setter(this);
            pPtr->__swConvertTo(TInterface::getInterfaceKey(), &setter);
        }
    }
    void releasePtr() {
        if( _ptr ) {
            _ptr->__swDecRef();
            _ptr = nullptr;
        }
    }
    template<typename Q> int assignPtr(Q* pPtr = nullptr) {
        if( _ptr != (TInterface*)pPtr ) {
            releasePtr();
            initPtr(pPtr);
        }
        return Error::Success;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_AutoPrt__h__