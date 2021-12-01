#ifndef __SimpleWork_IObject_h__
#define __SimpleWork_IObject_h__

__SimpleWork_Core_Namespace_Enter__


//
// 对象接口基类，注意：
//
//  1, 所有对象接口都需要直接或间接从这个接口派生
//  2, IObject的接口函数都不允许直接访问，避免引用计数错误，或者非法指针访问，请适用智能指针
//
SIMPLEWORK_INTERFACE_ENTER0(IObject, "sw.core.IObject", 011130)
protected:
    typedef struct __IPtrForceSaver {
        virtual int forceSetPtr(void* pPtr) = 0;
    }* FunPtrForceSaver;

private:
    virtual int __swAddRef() = 0;
    virtual int __swDecRef() = 0;
    virtual int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, FunPtrForceSaver funSaver) = 0;
    template<typename T> friend class TAutoPtr;

SIMPLEWORK_INTERFACE_LEAVE0

//
// IObjet类型的空指针定义，方便智能之指针类识别指针的类型
//
#define IObjectNullptr ((IObject*)nullptr)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IObject_h__