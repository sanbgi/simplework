#ifndef __SimpleWork_Core_CRefer_h__
#define __SimpleWork_Core_CRefer_h__

#include "core.h"
#include "CTaker.h"

__SimpleWork_Core_Namespace_Enter__


//
// 资源引用类，使用方法和CTaker基本相同，但允许多个对象共同持有资源(引用计数)
//
//    主要负责持有一个资源，资源可以转移，但不可以共同持有，主要为了实现申请资源和释放资源必须成对调用
// 时，在同时申请资源和定义资源的释放，具体使用方法：
//      
//      CRefer<char*> spArray(new char[10], [](char* pPtr) {delete pPtr;});
//      CRefer<char*> spRefer = spArray；
//
template<typename T> class CRefer {
public:
    typedef void(*FUnTake)(T);
    CRefer() {
        m_pBlock = nullptr;
    }
    CRefer(T hRes, FUnTake fUnTake) {
        m_pBlock = __BlockData::createBlock(hRes, fUnTake);
    }
    CRefer(const CRefer& src) {
        m_pBlock = src.m_pBlock;
        if(m_pBlock) {
            m_pBlock->addRef();
        }
    }
    const CRefer& operator = (const CRefer& src) {
        if(m_pBlock != src.m_pBlock ) {
            release();
            m_pBlock = src.m_pBlock;
            if(m_pBlock) {
                m_pBlock->addRef();
            }
        }
        return *this;
    }
    ~CRefer() {
        release();
    }
    void take(T hRes, FUnTake fUnTake) {
        release();
        m_pBlock = __BlockData::createBlock(hRes, fUnTake);
    }

public:
    T operator->() {
        return m_pBlock ? (T)m_pBlock->m_spTaker : nullptr;
    }
    T* operator &() {
        return m_pBlock ? (T*)m_pBlock->m_spTaker : nullptr;
    }
    operator T() {
        return m_pBlock ? (T)m_pBlock->m_spTaker : nullptr;
    }
    operator T() const {
        return m_pBlock ? (T)m_pBlock->m_spTaker : nullptr;
    }
    operator bool() {
        return m_pBlock != nullptr;
    }

private:
    void release() {
        if(m_pBlock) {
            m_pBlock->decRef();
            m_pBlock = nullptr;
        }
    }

private:
    struct __BlockData {
        int m_nRefCnt;
        CTaker<T> m_spTaker;
        static __BlockData* createBlock(T hRes, FUnTake fUnTake) {
            __BlockData* pBlock = new __BlockData();
            pBlock->m_nRefCnt = 1;
            pBlock->m_spTaker.take(hRes, fUnTake);
        }
        void decRef() {
            if( --m_nRefCnt == 0 ) {
                delete this;
            }
        }
        void addRef() {
            ++m_nRefCnt;
        }
    }* m_pBlock;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CRefer_h__