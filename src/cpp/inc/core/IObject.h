#ifndef __SimpleWork_IObject_h__
#define __SimpleWork_IObject_h__

#include "core.h"
#include "IVisitor.h"

__SimpleWork_Core_Namespace_Enter__

//
// 对象基础接口
//
struct IObject{
public:
    const static inline char* getInterfaceKey() { return "sw.core.IObject"; }
    static inline int getInterfaceVer() { return 211212; }

private:
    virtual int __swAddRef() = 0;
    virtual int __swDecRef() = 0;
    virtual int __swGetInterfacePtr(const char* szInterfaceKey, int nInterfaceVer, IVisitor<void*>& funSaver) = 0;
    template<typename T> friend class SPointer;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IObject_h__