#ifndef __SimpleWork_Core_CCallback_h__
#define __SimpleWork_Core_CCallback_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

struct ICallback {
    virtual int invoke() = 0;
};

class CCallback : ICallback {
public:
    typedef int (*FVisitor)();

public:
    int invoke() {
        return (*m_visitor)();
    }

public:
    CCallback(FVisitor callback) {
        m_visitor = callback;
    }

    operator ICallback*() {
        return this;
    }

private:
    FVisitor m_visitor;
};


template<typename P> class CPCallback : ICallback {
public:
    typedef int (*FVisitor)(P p);

public:
    int invoke() { 
        return (*m_visitor)(m_parameter);
    }

    CPCallback(P parameter, FVisitor callback) {
        m_parameter = parameter;
        m_visitor = callback;
    }
    operator ICallback*() {
        return this;
    }

private:
    P m_parameter;
    FVisitor m_visitor;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CCallback_h__