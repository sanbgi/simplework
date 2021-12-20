#ifndef __SimpleWork_Core_h__
#define __SimpleWork_Core_h__

//
//
//  Part1. 基础定义，包括
//      1.1, 命名域需要的宏
//
//
#define SIMPLEWORK_CORE_NAMESPACE sw::core
#define __SimpleWork_Core_Namespace_Enter__ namespace sw { namespace core {
#define __SimpleWork_Core_Namespace_Leave__ }}

//
//
//  Part2. 核心接口定义，包括：
//      2.1，定义接口需要的宏定义
//      2.2，核心接口
//          -- IObject, Object 接口定义基类及对应的智能指针
//      2.3, __getSimpleWork()获取全局模块对象
//
//
#define SIMPLEWORK_INTERFACECLASS_ENTER0(className) \
    class S##className {
#define SIMPLEWORK_INTERFACECLASS_ENTER(className, classKey) \
    class S##className {\
    public:\
    const static inline char* getClassKey() { return classKey; }

#define SIMPLEWORK_INTERFACECLASS_LEAVE(className) \
    private: \
        SIMPLEWORK_CORE_NAMESPACE::SPointer<IFace> __autoPtr;\
    public: \
        inline S##className(){}\
        inline S##className(IFace* pFace) { __autoPtr.setPtr(pFace); }\
        template<typename Q> inline S##className(const Q& src) { __autoPtr.setPtr(src.getPtr()); }\
        template<typename Q> inline const S##className& operator=(const Q& src) { __autoPtr.setPtr(src.getPtr()); return *this; }\
        inline IFace* getPtr() const { return __autoPtr.getPtr(); }\
        inline void setPtr(IFace* pPtr) { __autoPtr.setPtr(pPtr); }\
        inline IFace* operator->() const { return __autoPtr.getPtr(); }\
        inline operator bool() const { return __autoPtr; }\
        inline void release() { __autoPtr.release(); }\
    public:\
        static inline S##className wrapPtr(IFace* pPtr) {\
            S##className object;\
            object.setPtr(pPtr);\
            return object;\
        };\
    }; \
    typedef S##className::IFace I##className;

#define SIMPLEWORK_INTERFACE_ENTER(superInterfaceClass, interfaceKey, interfaceVer) \
    public:\
        struct IFace : public superInterfaceClass {\
            const static inline char* getInterfaceKey() { return interfaceKey; }\
            static inline int getInterfaceVer() { return interfaceVer; }

#define SIMPLEWORK_INTERFACE_LEAVE\
        };\

#include "SPointer.h"

//
//
//  Part3. 系统接口类定义：
//      3.1 SData 数据
//      3.2 SError 错误码
//      3.3 SModule 模块
//      3.4 SObject 对象
//      3.5 SFactory 工厂
//
//
#include "IObject.h"
#include "SObject.h"
#include "SData.h"
#include "SPipe.h"
#include "SError.h"
#include "SModule.h"
#include "SFactory.h"

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
        int __swGetInterfacePtr(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::IVisitor<void*>& visitor) { 
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE0 \
        return SError::ERRORTYPE_FAILURE; \
    };

#define SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass) \
    protected: \
        int __swGetInterfacePtr(const char* szInterfaceKey, int nInterfaceVer, SIMPLEWORK_CORE_NAMESPACE::IVisitor<void*>& visitor) { 
#define SIMPLEWORK_INTERFACE_ENTRY(TInterfaceClass) \
        if( strcmp(szInterfaceKey, TInterfaceClass::getInterfaceKey()) == 0 ) { \
            if( nInterfaceVer <= TInterfaceClass::getInterfaceVer() ) \
                return visitor.visit((void*)(TInterfaceClass*)this); \
            else \
                return SIMPLEWORK_CORE_NAMESPACE::SError::ERRORTYPE_FAILURE;\
        }
#define SIMPLEWORK_INTERFACE_ENTRY1(TInterface, TSuperInterface) \
        if( strcmp(szInterfaceKey, TSuperInterface::getInterfaceKey()) == 0 ) { \
            if( nInterfaceVer <= TSuperInterface::getInterfaceVer() ) \
                return visitor.visit((void*)(TSuperInterface*)(TInterface*)this); \
            else \
                return SIMPLEWORK_CORE_NAMESPACE::SError::ERRORTYPE_FAILURE;\
        }
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass) \
        return TSuperClass::__swGetInterfacePtr(szInterfaceKey, nInterfaceVer, visitor); \
    };

#include "CData.h"
#include "CTaker.h"
#include "CRefer.h"
#include "CPointer.h"
#include "CObject.h"
#include "CVisitor.h"
#include "CCallback.h"
#define SIMPLEWORK_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            SFactory spFactory = SIMPLEWORK_CORE_NAMESPACE::CObject::createFactory<className>(); \
            SIMPLEWORK_CORE_NAMESPACE::SModule::getSimpleWork()->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;

#define SIMPLEWORK_SINGLETON_FACTORY_REGISTER(className, classKey) \
    class __C##className##Register { \
    public: \
        __C##className##Register() { \
            SFactory spFactory = SIMPLEWORK_CORE_NAMESPACE::CObject::createFactory<className>(true); \
            __getSimpleWork(SIMPLEWORK_CORE_NAMESPACE::IModule::getInterfaceVer())->registerFactory(classKey, spFactory); \
        } \
    } __g##className##Register;


#ifdef SIMPLEWORK_WITHOUTAPI
    #include "CModule.h"
    #define SIMPLEWORK_MODULE_REGISTER \
    __SimpleWork_API__ SIMPLEWORK_CORE_NAMESPACE::SModule& __getSimpleWork(int nCompatibleVer) { \
        static SModule s_spModule = \
            SIMPLEWORK_CORE_NAMESPACE::CObject::createObject<SIMPLEWORK_CORE_NAMESPACE::CModule>(); \
        return s_spModule; \
    }
#else//SIMPLEWORK_WITHOUTAPI
    #define SIMPLEWORK_MODULE_REGISTER
#endif//SIMPLEWORK_WITHOUTAPI

#endif//__SimpleWork_Core_h__