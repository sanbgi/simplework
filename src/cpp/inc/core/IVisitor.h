#ifndef __SimpleWork_Core_IVisitor_h__
#define __SimpleWork_Core_IVisitor_h__

#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
// 访问者模式接口
//
template<typename T, typename R=int> struct IVisitor {

    virtual R visit(T data) = 0;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_IVisitor_h__