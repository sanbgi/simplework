#ifndef __SimpleWork_Core_CPointer_h__
#define __SimpleWork_Core_CPointer_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// 资源引用类，使用方法和CPointer基本相同，但允许多个对象共同持有资源(引用计数)
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
    void take(T* pPtr, SObject spPtrTaker) {
        m_pPtr = pPtr;
        m_spPtrTaker = spPtrTaker;
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

private:
    SObject m_spPtrTaker;
    T* m_pPtr;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CPointer_h__