#ifndef __SimpleWork_Core_IVisitor_h__
#define __SimpleWork_Core_IVisitor_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

template<typename T> struct IVisitor {
    virtual int visit(T data) = 0;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_IVisitor_h__