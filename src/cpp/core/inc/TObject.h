#ifndef __SimpleWork_TObject__h__
#define __SimpleWork_TObject__h__

__SimpleWork_Core_Namespace_Enter__

template<typename TClass, typename TInterface> struct TObject {
public:
    operator bool() {
        return m_autoPtr;
    }

public:
    template<typename Q> static TClass wrapPtr(const Q& sp) {
        TClass ret;
        ret.m_autoPtr = sp;
        return ret;
    }
    template<typename Q> static TClass wrapPtr(const Q* pSrc) {
        TClass ret;
        ret.m_autoPtr = pSrc;
        return ret;
    }
    const TAutoPtr<TInterface> & getAutoPtr() const {
        return m_autoPtr;
    }

protected:
    TAutoPtr<TInterface> m_autoPtr;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_TObject__h__