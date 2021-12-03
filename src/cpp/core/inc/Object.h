#ifndef __SimpleWork_Object_h__
#define __SimpleWork_Object_h__

__SimpleWork_Core_Namespace_Enter__

//
// 对象接口基类，注意：
//
//  1, 所有对象接口都需要直接或间接从这个接口派生
//  2, IObject的接口函数都不允许直接访问，避免引用计数错误，或者非法指针访问，请适用智能指针
//
SIMPLEWORK_INTERFACECLASS_ENTER(Object)
    SIMPLEWORK_INTERFACE_ENTER0("sw.core.IObject", 211202)
        private:
        virtual int __swAddRef() = 0;
        virtual int __swDecRef() = 0;
        virtual int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, FunPtrForceSaver funSaver) = 0;
        template<typename T> friend class TAutoPtr;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename TFactory=Module> static Object createObject( const char* szClassKey) {
        return TFactory::createObject(szClassKey);
    }
    template<typename TFactory=Module> static Object createFactory( const char* szClassKey) {
        return TFactory::createFactory(szClassKey);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Object)

//
// IObjet类型的空指针定义，方便智能之指针类识别指针的类型
//
#define IObjectNullptr Object()

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Object_h__