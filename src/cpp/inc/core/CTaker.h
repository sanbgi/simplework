#ifndef __SimpleWork_Core_CTaker_h__
#define __SimpleWork_Core_CTaker_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// 资源占有类
//
//    主要负责持有一个资源，资源可以转移，但不可以共同持有，主要为了实现申请资源和释放资源必须成对调用
// 时，在同时申请资源和定义资源的释放，具体使用方法：
//      
//      CTaker<char*> spArray(new char[10], [](char* pPtr) {delete pPtr;});
//      CTaker<char*> spTaker；
//      spTaker.take(spArray);  //这个操作后，spArray里面的值被转移到spTaker中了。
//
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
        release();
        m_hRes = hRes;
        m_fUnTake = fUnTake;
    }
    T untake() {
        T ret = m_hRes;
        m_hRes = nullptr;
        m_fUnTake = nullptr;
        return ret;
    }
    void release() {
        if(m_fUnTake && m_hRes != nullptr) {
            (*m_fUnTake)(m_hRes);
            m_fUnTake = nullptr;
            m_hRes = nullptr;
        }
    }

private:
    //不允许使用赋值函数和赋值构造函数
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

    operator T() {
        return m_hRes;
    }
    operator T() const {
        return m_hRes;
    }
    operator bool() {
        return m_hRes != nullptr;
    }

private:
    T m_hRes;
    FUnTake m_fUnTake;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_AutoPtr_h__