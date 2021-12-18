#ifndef __SimpleWork_Core_CPointer_h__
#define __SimpleWork_Core_CPointer_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// CObject类型的指针持有类
//
template<typename T> class CPointer {
public:
    CPointer() {
        m_pPtr = nullptr;
    }
    CPointer(T* pPtr, SObject& spPtrTaker) {
        m_pPtr = pPtr;
        m_spPtrTaker = spPtrTaker;
    }
    CPointer(const CPointer& src) {
        m_pPtr = src.m_pPtr;
        m_spPtrTaker = src.m_spPtrTaker;
    }
    const CPointer& operator = (const CPointer& src) {
        m_pPtr = src.m_pPtr;
        m_spPtrTaker = src.m_spPtrTaker;
        return *this;
    }
    ~CPointer() {
        release();
    }
    T* getPtr() const {
        return m_pPtr;
    }
    void release() {
        m_pPtr = nullptr;
        m_spPtrTaker.release();
    }

public:
    T* operator->() {
        return m_pPtr;
    }
    operator T*() {
        return m_pPtr;
    }
    operator T*() const {
        return m_pPtr;
    }
    operator bool() {
        return m_pPtr != nullptr;
    }
    operator SObject&() {
        return m_spPtrTaker;
    }

public:
    SObject& getObject() {
        return m_spPtrTaker;
    }

private:
    void take(T* pPtr, IObject* pTaker) {
        m_pPtr = pPtr;
        m_spPtrTaker.setPtr(pTaker);
    }

private:
    SObject m_spPtrTaker;
    T* m_pPtr;

    friend class CObject;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CPointer_h__