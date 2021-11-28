#ifndef __SimpleWork_Core_h__
#define __SimpleWork_Core_h__

//
//
//  Part1. 基础定义，包括
//      1.1, 命名域需要的宏
//      1.2, 错误码, Error
//      1.3, 智能指针定义, SmartPtr
//
//
#define SIMPLEWORK_CORE_NAMESPACE sw
#define __SimpleWork_Core_Namespace_Enter__ namespace SIMPLEWORK_CORE_NAMESPACE {
#define __SimpleWork_Core_Namespace_Leave__ }
#include <cstring>
#include "Error.h"
#include "SmartPtr.h"

//
//
//  Part2. 核心接口定义，包括：
//      2.1，定义接口需要的宏定义
//      2.2，核心接口
//          -- IObject, IObjectPtr 接口定义基类及对应的智能指针
//          -- IFactory, IFactoryPtr 工厂接口及对应的智能指针
//          -- IModule, IModulePtr 模块接口及对应的智能指针
//
//
#define SIMPLEWORK_INTERFACE_ENTER0(namespaceName, interfaceName) \
    struct interfaceName; \
    typedef SmartPtr<interfaceName> interfaceName##Ptr; \
    struct interfaceName { \
        static const char* getInterfaceKey() { return #namespaceName"."#interfaceName; }
#define SIMPLEWORK_INTERFACE_LEAVE0 };

//
// 接口宏定义
//
#define SIMPLEWORK_INTERFACE_ENTER(namespaceName, interfaceName, superInterfaceName)  \
    struct interfaceName; \
    typedef SmartPtr<interfaceName> interfaceName##Ptr; \
    struct interfaceName : superInterfaceName { \
        static const char* getInterfaceKey() { return #namespaceName"."#interfaceName; }
#define SIMPLEWORK_INTERFACE_LEAVE };

#include "IObject.h"
#include "IFactory.h"
#include "IModule.h"

//
//
//  Part3. 全局函数定义，包括：
//      3.1 getSimpleWorkModule获取当前模块接口
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

#include "ICoreApi.h"
#include "CoreApi.h"

//
//
//  Part4. 自定义对象类相关支持，包括：
//      4.1 对象类的接口定义宏
//          SIMPLEWORK_INTERFACE_ENTRY_ENTER(superClass)
//              SIMPLEWORK_INTERFACE_ENTRY(interface)
//          SIMPLEWORK_INTERFACE_ENTRY_LEAVE(superClass)
//      4.2 对象类基类定义，CObject
//      4.3 对象类工厂类定义，CFactory
//
//

//
// 工厂类基础宏定义
//
#define SIMPLEWORK_INTERFACE_ENTRY_ENTER0 \
protected: \
    int __swConvertTo(const char* szInterfaceKey, IPtrForceSaver* pTarget) { 
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE0 \
    return Error::Failure; \
};
#define SIMPLEWORK_INTERFACE_ENTRY_ENTER(TSuperClass) \
protected: \
    int __swConvertTo(const char* szInterfaceKey, IPtrForceSaver* pTarget) { 
#define SIMPLEWORK_INTERFACE_ENTRY(TInterface) \
        if( strcmp(szInterfaceKey, TInterface::getInterfaceKey()) == 0 ) { \
            return pTarget->forceSetPtr((void*)(TInterface*)this); \
        }
#define SIMPLEWORK_INTERFACE_ENTRY_LEAVE(TSuperClass) \
        return TSuperClass::__swConvertTo(szInterfaceKey, pTarget); \
    };

#include "CObject.h"
#include "CFactory.h"

//
//
//  Part5. 指针封装类定义支持，指针封装类主要用户隐藏指针细节，让使用者可以直接像一个类一
//      样使用对象，这些类其实就是对接口调用的封装。目前的实现大多是直接从智能指针派生
//      1, 定义封装类需要的宏
//              SIMPLEWORK_CLASS_ENTER(name)
//              SIMPLEWORK_CLASS_LEAVE
//
//

#define SIMPLEWORK_CLASS_ENTER(x) \
class x : public sw::SmartPtr<I##x> { \
public: \
    x() : SmartPtr<I##x>() {} \
    x(const x& src) : SmartPtr<I##x>(src) {} \
    template<typename Q> x(const SmartPtr<Q>& src) : SmartPtr<I##x>(src) {};\
    template<typename Q> x(Q* pSrc) : SmartPtr<Q>(pSrc) {};
#define SIMPLEWORK_CLASS_LEAVE };

#include "Object.h"
#include "Factory.h"
#include "Module.h"

#endif//__SimpleWork_Core_h__