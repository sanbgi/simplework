#ifndef __SimpleWork_SPointer__h__
#define __SimpleWork_SPointer__h__

#include "core.h"
#include "IVisitor.h"

__SimpleWork_Core_Namespace_Enter__

//
// 对象的智能指针定义，仅适用于从IObject派生的接口，目前建议不要直接使用
//
template<typename TInterface> class SPointer {

public:
    inline SPointer(){
        m_ptr = nullptr;
    }
    inline SPointer(const SPointer& src) : m_ptr(nullptr)  {
        initPtr(src.m_ptr);
    }
    inline SPointer& operator=(const SPointer& src) {
        assignPtr(src.m_ptr);
        return *this;
    }
    inline ~SPointer() {
        releasePtr();
    }
    template<typename Q> inline SPointer(Q* pPtr) : m_ptr(nullptr) {
       initPtr(pPtr);
    }
    template<typename Q> inline SPointer(const Q& src) : m_ptr(nullptr) {
        initPtr(src.getPtr());
    }
    template<typename Q> inline const SPointer& operator=(Q* pSrc) {
        assignPtr(pSrc);
        return *this;
    }
    template<typename Q> inline const SPointer& operator=(const Q& src) {
        assignPtr(src.getPtr());
        return *this;
    }

public:
    inline TInterface* getPtr() const { 
        return m_ptr; 
    }
    template<typename Q> inline int setPtr(Q* pPtr) {
        return assignPtr(pPtr);
    }

public:
    inline bool isNullptr() const { 
        return m_ptr == nullptr; 
    }
    inline TInterface* operator->() const {
        return m_ptr; 
    }
    inline operator bool() const{
        return m_ptr != nullptr;
    }
    inline operator TInterface*() const {
        return m_ptr;
    }
    inline void release() {
        releasePtr();
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
            //
            // 如果指针可以安全的转化为目标类型指针时，直接转化，无需查询接口
            //
            TInterface* pDest = dynamic_cast<TInterface*>(pPtr);
            if(pDest) {
                initPtr(pDest);
            }else{
                struct CPtrVisitor : public IVisitor<void*> {
                    CPtrVisitor(SPointer* pAutoPtr) : _pPtr(pAutoPtr){}
                    int visit(void* pPtr) { return _pPtr->assignPtr((TInterface*)pPtr); }
                    SPointer* _pPtr;
                }visitor(this);
                pPtr->__swGetInterfacePtr( TInterface::getInterfaceKey(), TInterface::getInterfaceVer(), visitor);
            }
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
        return 0;
    }
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_SPointer__h__