#ifndef __SimpleWork_Core_CVisitor_h__
#define __SimpleWork_Core_CVisitor_h__

#include "core.h"
#include "IVisitor.h"

__SimpleWork_Core_Namespace_Enter__

//
// 数据访问类
//
template<typename T, typename R=void, typename CB=R(*)(T data)> class CVisitor : public IVisitor<T,R> {
public:
    R visit(T data) {
        return m_visitor(data);
    }

public:
    CVisitor(CB visitor) {
        m_visitor = visitor;
    }

    operator IVisitor<T,R>*() {
        return this;
    }

private:
    CB m_visitor;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CVisitor_h__