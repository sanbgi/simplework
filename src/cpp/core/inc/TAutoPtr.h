#ifndef __SimpleWork_AutoPrt__h__
#define __SimpleWork_AutoPrt__h__

__SimpleWork_Core_Namespace_Enter__

//
// 对象的智能指针定义，仅适用于从IObject派生的接口
//
template<typename TInterface> struct TAutoPtr {

public:
    TAutoPtr(){
        m_ptr = nullptr;
    }
    TAutoPtr(const TAutoPtr& src) : m_ptr(nullptr)  {
        initPtr(src.m_ptr);
    }
    TAutoPtr& operator=(const TAutoPtr& src) {
        assignPtr(src.m_ptr);
        return *this;
    }
    ~TAutoPtr() {
        releasePtr();
    }
    TAutoPtr(const char* szClassKey) {
        *this = getSimpleWork()->createObject(szClassKey);
    }

    template<typename Q> TAutoPtr(Q* pPtr) : m_ptr(nullptr) {
       initPtr(pPtr);
    }
    template<typename Q> TAutoPtr(const Q& src) : m_ptr(nullptr) {
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
        return m_ptr; 
    }
    TInterface& getRef() const {
        return *m_ptr;
    }

public:
    bool isNullptr() const { 
        return m_ptr == nullptr; 
    }
    TInterface* operator->() const {
        return m_ptr; 
    }
    operator bool(){
        return m_ptr != nullptr;
    }
    operator TInterface*() const {
        return m_ptr;
    }

//
// 
//  私有成员及函数区域
// 
//
private:
    TInterface* m_ptr = nullptr;

private:
    void initPtr(TInterface* pPtr = nullptr) {
        if( pPtr ) {
            m_ptr = pPtr;
            m_ptr->__swAddRef();
        }
    }
    template<typename Q> void initPtr(Q* pPtr = nullptr) {
        if( pPtr ) {
            struct CForceSetter : public IObject::__IPtrForceSaver {
                CForceSetter(TAutoPtr* pAutoPtr) : _pPtr(pAutoPtr){}
                int forceSetPtr(void* pPtr) { return _pPtr->assignPtr((TInterface*)pPtr); }
                TAutoPtr* _pPtr;
            }setter(this);
            pPtr->__swConvertTo(TInterface::getInterfaceKey(), TInterface::getInterfaceVer(), &setter);
        }
    }
    void releasePtr() {
        if( m_ptr ) {
            m_ptr->__swDecRef();
            m_ptr = nullptr;
        }
    }
    template<typename Q> int assignPtr(Q* pPtr = nullptr) {
        if( m_ptr != (TInterface*)pPtr ) {
            releasePtr();
            initPtr(pPtr);
        }
        return Error::Success;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_AutoPrt__h__