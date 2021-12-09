#ifndef __SimpleWork_Object_h__
#define __SimpleWork_Object_h__

__SimpleWork_Core_Namespace_Enter__

class Module;
class Factory;

//
// 对象接口基类，注意：
//
//  1, 所有对象接口都需要直接或间接从这个接口派生
//  2, IObject的接口函数都不允许直接访问，避免引用计数错误，或者非法指针访问，请适用智能指针
//
SIMPLEWORK_INTERFACECLASS_ENTER0(Object)

    SIMPLEWORK_INTERFACE_ENTER0("sw.IObject", 211202)
    private:
        virtual int __swAddRef() = 0;
        virtual int __swDecRef() = 0;
        virtual int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, __FunPtrForceSaver funSaver) = 0;
        template<typename T> friend class __CPointer;
    SIMPLEWORK_INTERFACE_LEAVE

public:
    template<typename TModule=Module> static inline Object createObject(const char* szClassKey) {
        return TModule::createObject(szClassKey);
    }
    template<typename TModule=Module> static inline Object createFactory(const char* szClassKey) {
        return TModule::createFactory(szClassKey);
    }
    template<typename TObject, typename TFactory=Factory, typename TModule=Module> static Object createObject() {
        static TFactory s_factory =  TModule::createFactory(TObject::getClassKey());
        return s_factory->createObject();
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Object)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Object_h__