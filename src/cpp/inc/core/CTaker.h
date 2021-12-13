#ifndef __SimpleWork_Core_CTaker_h__
#define __SimpleWork_Core_CTaker_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

template<typename T> class CTaker {
public:
    typedef void(*FUnTake)(T);
    CTaker() {
        m_hRes = nullptr;
        m_fUnTake = nullptr;
    }
    CTaker(T hRes, FUnTake fUnTake) {
        m_hRes = hRes;
        m_fUnTake = fUnTake;
    }
    ~CTaker() {
        release();
    }
    void take(CTaker& rRes) {
        m_hRes = rRes.m_hRes;
        m_fUnTake = rRes.m_fUnTake;
        rRes.m_hRes = nullptr;
        rRes.m_fUnTake = nullptr;
    }
    void take(T hRes, FUnTake fUnTake) {
        untake();
        m_hRes = hRes;
        m_fUnTake = fUnTake;
    }
    void untake() {
        release();
    }

private:
    CTaker(const CTaker& src) {
    }
    const CTaker& operator = (const CTaker& src) {
        return *this;
    }

public:
    T operator->() {
        return m_hRes;
    }
    T* operator &() {
        return &m_hRes;
    }

    operator T () {
        return m_hRes;
    }
    operator bool() {
        return m_hRes != nullptr;
    }

    T getRes() {
        return m_hRes;
    }

private:
    void release() {
        if(m_fUnTake && m_hRes != nullptr) {
            (*m_fUnTake)(m_hRes);
            m_fUnTake = nullptr;
            m_hRes = nullptr;
        }
    }

private:
    T m_hRes;
    FUnTake m_fUnTake;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_AutoPtr_h__