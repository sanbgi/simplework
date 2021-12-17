#ifndef __SimpleWork_Core_CVisitor_h__
#define __SimpleWork_Core_CVisitor_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

template<typename T> struct IVisitor {
    virtual int visit(T data) = 0;
};

template<typename T> class CVisitor : IVisitor<T> {
public:
    typedef int (*FVisitor)(T data);

public:
    int visit(T data) {
        return (*m_visitor)(data);
    }

public:
    CVisitor(FVisitor visitor) {
        m_visitor = visitor;
    }

    operator IVisitor<T>*() {
        return this;
    }

private:
    FVisitor m_visitor;
};


template<typename P, typename T> class CPVisitor : IVisitor<T> {
public:
    typedef int (*FVisitor)(P p, T data);

public:
    int visit(T data) { 
        return (*m_visitor)(m_parameter, data);
    }

    CPVisitor(P parameter, FVisitor visitor) {
        m_parameter = parameter;
        m_visitor = visitor;
    }
    operator IVisitor<T>*() {
        return this;
    }

private:
    P m_parameter;
    FVisitor m_visitor;
};

template<typename P, typename Q, typename T> class CPQVisitor : IVisitor<T> {
public:
    typedef int (*FVisitor)(P p, Q q, T data);

public:
    int visit(T data) { 
        return (*m_visitor)(m_p, m_q, data);
    }

    CPQVisitor(P p, Q q, FVisitor visitor) {
        m_p = p;
        m_q = q;
        m_visitor = visitor;
    }
    operator IVisitor<T>*() {
        return this;
    }

private:
    P m_p;
    Q m_q;
    FVisitor m_visitor;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CVisitor_h__