#ifndef __SimpleWork_IObject_h__
#define __SimpleWork_IObject_h__

namespace SimpleWork {


//
// 对象接口基类，注意：
//
//  1, 所有对象接口都需要直接或间接从这个接口派生
//  2, 接口函数都不允许直接访问，避免引用计数错误，或者非法指针访问，请适用智能指针
//
__SimpleWork_SuperInterface_Enter__(SimpleWork, IObject)

protected:
    //
    // 指针强制设置接口，由于设置的是强制转化后的地址，所以，接口不安全，也不能被外界使用
    //
    struct IPointerForceSetter {
        virtual int forceSetPtr(void* pPtr) = 0;
    };

private:
    virtual int addRef() = 0;
    virtual int decRef() = 0;
    virtual int convertTo(const char* szInterfaceKey, IPointerForceSetter* pSaver) = 0;
    template<typename T> friend class SmartPtr;

__SimpleWork_SuperInterface_Leave__

}//namespace SimpleWork

#endif//__SimpleWork_IObject_h__