#ifndef __SimpleWork_Core_h__
#define __SimpleWork_Core_h__

//
//
//  Part1. 基础定义，包括
//      1.1, 命名域需要的宏
//      1.2, 数据，Data
//      1.3, 错误码, Error
//
//
#define SIMPLEWORK_CORE_NAMESPACE sw::core
#define __SimpleWork_Core_Namespace_Enter__ namespace sw { namespace core {
#define __SimpleWork_Core_Namespace_Leave__ }}
#include "Data.h"
#include "Error.h"


//
//
//  Part2. 核心接口定义，包括：
//      2.1，定义接口需要的宏定义
//      2.2，核心接口
//          -- IObject, Object 接口定义基类及对应的智能指针
//          -- IFactory, Factory 工厂接口及对应的智能指针
//      2.3, getSimpleWork()获取全局模块对象
//
//
#define SIMPLEWORK_INTERFACECLASS_ENTER0(className) \
    class className {
#define SIMPLEWORK_INTERFACECLASS_ENTER(className, classKey) \
    class className {\
    public:\
    const static inline char* getClassKey() { return classKey; }

#define SIMPLEWORK_INTERFACECLASS_LEAVE(className) \
    private: \
        SIMPLEWORK_CORE_NAMESPACE::__CPointer<IFace> __autoPtr;\
    public: \
        inline className(){}\
        inline className(IFace* pFace) { \
            __autoPtr.setPtr(pFace);\
        }\
        inline className(const className& src) {\
            __autoPtr.setPtr(src.getPtr());\
        }\
        inline const className& operator=(const className& src) {\
            __autoPtr.setPtr(src.getPtr()); \
            return *this; \
        }\
        template<typename Q> inline className(const Q& src) {\
            __autoPtr.setPtr(src.getPtr());\
        }\
        template<typename Q> inline const className& operator=(const Q& src) {\
            __autoPtr.setPtr(src.getPtr());\
            return *this;\
        }\
    public:\
        inline IFace* getPtr() const { \
            return __autoPtr.getPtr(); \
        }\
        inline void setPtr(IFace* pFactory) {\
            __autoPtr.setPtr(pFactory);\
        }\
        inline IFace* operator->() const {\
            return __autoPtr.getPtr(); \
        }\
        inline operator bool() const {\
            return __autoPtr;\
        }\
    public:\
        static inline className wrapPtr(IFace* pObject) {\
            className object;\
            object.setPtr(pObject);\
            return object;\
        };\
    }; \
    typedef className::IFace I##className;

#define SIMPLEWORK_INTERFACE_ENTER(superInterfaceClass, interfaceKey, interfaceVer) \
    public:\
        struct IFace : public superInterfaceClass {\
            const static inline char* getInterfaceKey() { return interfaceKey; }\
            static inline int getInterfaceVer() { return interfaceVer; }

#define SIMPLEWORK_INTERFACE_ENTER0(interfaceKey, interfaceVer) \
    public:\
        struct IFace {\
            const static inline char* getInterfaceKey() { return interfaceKey; }\
            static inline int getInterfaceVer() { return interfaceVer; }

#define SIMPLEWORK_INTERFACE_LEAVE\
        };\

#include "__CPointer.h"
#include "Object.h"
#include "Factory.h"
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
        int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::__FunPtrForceSaver pTarget) { 
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE0 \
        return Error::ERRORTYPE_FAILURE; \
    };

#define SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass) \
    protected: \
        int __swConvertTo(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::__FunPtrForceSaver pTarget) { 
#define SIMPLEWORK_INTERFACE_ENTRY(TInterfaceClass) \
        if( strcmp(szInterfaceKey, TInterfaceClass::getInterfaceKey()) == 0 ) { \
            if( nInterfaceVer <= TInterfaceClass::getInterfaceVer() ) \
                return pTarget->forceSetPtr((void*)(TInterfaceClass*)this); \
            else \
                return SIMPLEWORK_CORE_NAMESPACE::Error::ERRORTYPE_FAILURE;\
        }
#define SIMPLEWORK_INTERFACE_ENTRY1(TInterface, TSuperInterface) \
        if( strcmp(szInterfaceKey, TSuperInterface::getInterfaceKey()) == 0 ) { \
            if( nInterfaceVer <= TSuperInterface::getInterfaceVer() ) \
                return pTarget->forceSetPtr((void*)(TSuperInterface*)(TInterface*)this); \
            else \
                return SIMPLEWORK_CORE_NAMESPACE::Error::ERRORTYPE_FAILURE;\
        }
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass) \
        return TSuperClass::__swConvertTo(szInterfaceKey, nInterfaceVer, pTarget); \
    };

#include "CObject.h"
#define SIMPLEWORK_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            Factory spFactory = SIMPLEWORK_CORE_NAMESPACE::CObject::createFactory<className>(); \
            SIMPLEWORK_CORE_NAMESPACE::Module::getSimpleWork()->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;

#define SIMPLEWORK_SINGLETON_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            Factory spFactory = SIMPLEWORK_CORE_NAMESPACE::CObject::createFactory<className>(true)); \
            SIMPLEWORK_CORE_NAMESPACE::Module::getSimpleWork()->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;


#ifdef SIMPLEWORK_WITHOUTAPI
    #include "__CModule.h"
    #define SIMPLEWORK_MODULE_REGISTER(moduleKey) \
    SIMPLEWORK_MODULE_EXPORT Module& getSimpleWork() { \
        static Module s_spModule = \
            SIMPLEWORK_CORE_NAMESPACE::CObject::createObject<SIMPLEWORK_CORE_NAMESPACE::__CModule>(); \
        return s_spModule; \
    }
#else//SIMPLEWORK_WITHOUTAPI
    #define SIMPLEWORK_MODULE_REGISTER(moduleKey)
#endif//SIMPLEWORK_WITHOUTAPI

#endif//__SimpleWork_Core_h__