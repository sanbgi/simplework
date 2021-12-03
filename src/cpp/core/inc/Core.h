#ifndef __SimpleWork_Core_h__
#define __SimpleWork_Core_h__

//
//
//  Part1. 基础定义，包括
//      1.1, 命名域需要的宏
//      1.2, 错误码, Error
//      1.3, 智能指针定义, TAutoPtr
//      1.4，核心接口申明
//
//
#define SIMPLEWORK_CORE_NAMESPACE sw::core
#define __SimpleWork_Core_Namespace_Enter__ namespace SIMPLEWORK_CORE_NAMESPACE {
#define __SimpleWork_Core_Namespace_Leave__ }
#include "Error.h"

__SimpleWork_Core_Namespace_Enter__
    struct Object;
    struct Module;
    struct Factory;
    struct CoreApi;
    typedef struct __IPtrForceSaver {
        virtual int forceSetPtr(void* pPtr) = 0;
    }* FunPtrForceSaver;
__SimpleWork_Core_Namespace_Leave__

//
//
//  Part2. 核心接口定义，包括：
//      2.1，定义接口需要的宏定义
//      2.2，核心接口
//          -- IObject, Object 接口定义基类及对应的智能指针
//          -- IFactory, Factory 工厂接口及对应的智能指针
//          -- IModule, Module 模块接口及对应的智能指针
//
//
#define SIMPLEWORK_INTERFACECLASS_ENTER(className) \
    class className {

#define SIMPLEWORK_INTERFACE_LEAVE(className) \
    private: \
        TAutoPtr<IFace> m_autoPtr;\
    public: \
        className(){}\
        template<typename Q> className(const Q& src) {\
            m_autoPtr = src.getPtr();\
        }\
        template<typename Q> const className& operator=(const Q& src) {\
            m_autoPtr = src.getPtr();\
            return *this;\
        }\
    public:\
        IFace* getPtr() const { \
            return m_autoPtr.getPtr(); \
        }\
        void setPtr(IFace* pFactory) {\
            m_autoPtr = pFactory;\
        }\
        IFace* operator->() const {\
            return m_autoPtr.getPtr(); \
        }\
        operator bool() const {\
            return m_autoPtr;\
        }\
    public:\
        static className wrapPtr(IFace* pObject) {\
            className object;\
            object.setPtr(pObject);\
            return object;\
        };\
    }; \
    typedef className::IFace I##className;

#define SIMPLEWORK_INTERFACE_ENTER(superInterfaceClass, interfaceKey, interfaceVer) \
    public:\
        struct IFace : public superInterfaceClass {\
            const static char* getInterfaceKey() { return interfaceKey; }\
            static int getInterfaceVer() { return interfaceVer; }

#define SIMPLEWORK_INTERFACE_ENTER0(interfaceKey, interfaceVer) \
    public:\
        struct IFace {\
            const static char* getInterfaceKey() { return interfaceKey; }\
            static int getInterfaceVer() { return interfaceVer; }

#define SIMPLEWORK_OBJECT_INTERFACE_LEAVE\
        };\

#include "TAutoPtr.h"
#include "Object.h"
#include "Factory.h"

//
//
//  Part3. 全局函数定义，包括：
//      3.1 getSimpleWork获取当前模块接口
//      3.2 getCoreModule获取核心模块接口
//
//
#ifndef __SimpleWork_API__
    #if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
        #define __SimpleWork_API__ __declspec(dllimport)
    #else
        #define __SimpleWork_API__
    #endif
#endif//__SimpleWork_API__

//
// 兼容版本号定义，头文件定义需要和编译好的库定义一致，每一次头文件更改后，务必及时修改此版本号
//
#ifndef SIMPLEWORK_COMPATIBLE_VER
#define SIMPLEWORK_COMPATIBLE_VER 211124
#endif//SIMPLEWORK_COMPATIBLE_VER

#include "CoreApi.h"
#include "Module.h"

//
//
//  Part4. 自定义对象、工厂、模块相关支持，包括：
//      4.1 对象类的接口定义宏
//          SIMPLEWORK_INTERFACE_ENTRY_ENTER(superClass)
//              SIMPLEWORK_INTERFACE_ENTRY(interface)
//          SIMPLEWORK_INTERFACE_ENTRY_LEAVE(superClass)
//      4.2 注册类工厂宏 
//              普通工厂SIMPLEWORK_FACTORY_REGISTER(classType, szClassKey)
//              单例工厂SIMPLEWORK_SINGLETONFACTORY_REGISTER(classType, szClassKey)
//      4.3 注册模块宏 SIMPLEWORK_MODULE_REGISTER(szModuleKey)
//      4.4 对象类基类定义，CObject
//      4.5 对象类工厂类定义，CFactory
//
//

//
// 工厂类基础宏定义
//
#include <cstring>
#define SIMPLEWORK_INTERFACE_ENTRY_ENTER0 \
    protected: \
        sw::core::Object::IFace * __swGetIObject() { return (sw::core::Object::IFace*)this; } \
        int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::FunPtrForceSaver pTarget) { 
    #define SIMPLEWORK_INTERFACE_ENTRY_LEAVE0 \
        return Error::Failure; \
    };
#define SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass) \
    protected: \
        sw::core::Object::IFace * __swGetIObject() { return TSuperClass::__swGetIObject(); } \
        int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::FunPtrForceSaver pTarget) { 
#define SIMPLEWORK_INTERFACE_ENTRY(TInterfaceClass) \
        if( strcmp(szInterfaceKey, TInterfaceClass::getInterfaceKey()) == 0 ) { \
            if( nInterfaceVer <= TInterfaceClass::getInterfaceVer() ) \
                return pTarget->forceSetPtr((void*)(TInterfaceClass*)this); \
            else \
                return Error::Failure;\
        }
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass) \
        return TSuperClass::__swConvertTo(szInterfaceKey, nInterfaceVer, pTarget); \
    };

#include "CObject.h"
#define SIMPLEWORK_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            Factory spFactory = CObject::createFactory<className>(); \
            getSimpleWork()->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;

#define SIMPLEWORK_SINGLETON_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            Factory spFactory = CObject::createFactory<className>(true)); \
            getSimpleWork()->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;

#ifndef SIMPLEWORK_MODULE_REGISTER
    #ifndef SIMPLEWORK_WITHOUTAPI
        #define SIMPLEWORK_MODULE_REGISTER(moduleKey) \
            SIMPLEWORK_MODULE_EXPORT SIMPLEWORK_CORE_NAMESPACE::Module& getSimpleWork() { \
                static SIMPLEWORK_CORE_NAMESPACE::Module s_spModule = SIMPLEWORK_CORE_NAMESPACE::getCoreApi()->createModule(moduleKey); \
                return s_spModule; \
            }
    #else//SIMPLEWORK_WITHOUTAPI
        #include "CModule.h"
        #define SIMPLEWORK_MODULE_REGISTER(moduleKey) \
        SIMPLEWORK_MODULE_EXPORT Module& getSimpleWork() { \
            static Module s_spModule = \
                SIMPLEWORK_CORE_NAMESPACE::CObject::createObject<SIMPLEWORK_CORE_NAMESPACE::CModule>(); \
            return s_spModule; \
        }
    #endif//SIMPLEWORK_WITHOUTAPI
#endif//SIMPLEWORK_MODULE_REGISTER

#endif//__SimpleWork_Core_h__