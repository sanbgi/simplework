#ifndef __SimpleWork_Av_Av_h__
#define __SimpleWork_Av_Av_h__

#include "../inc/av/av.h"

template<typename Q> class CAutoPointer {
public:
    typedef void (*FUN)(Q*);
    CAutoPointer(Q* pQ, FUN fun) {
        m_ptr = pQ;
        m_fun = fun;
    }
    ~CAutoPointer() {
        if(m_ptr) {
            (*m_fun)(m_ptr);
        }
    }
    Q* operator->() {
        return m_ptr;
    }
    operator Q*() {
        return m_ptr;
    }
    Q* detach() {
        Q* ptr = m_ptr;
        m_ptr = nullptr;
        return ptr;
    }

private:
    Q* m_ptr;
    FUN m_fun;
};

#endif//#define __SimpleWork_Av_Av_h__