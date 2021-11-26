#ifndef __SimpleWork_Core_h__
#define __SimpleWork_Core_h__

//
// 函数输出需要的宏定义
//
#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #ifdef __SimpleWork_Module__
        #define __SimpleWork_API__ __declspec(dllexport)
    #else
        #define __SimpleWork_API__ __declspec(dllimport)
    #endif
#else
    #define __SimpleWork_API__
#endif

//
// 兼容版本号定义，头文件定义需要和编译好的库定义一致，每一次头文件更改后，务必及时修改此版本号
//
#ifndef SIMPLEWORK_COMPATIBLE_VER
#define SIMPLEWORK_COMPATIBLE_VER 211124
#endif//SIMPLEWORK_COMPATIBLE_VER

#include <cstring>
#include "Error.h"

//
// 超级接口宏定义
//
#define __SimpleWork_SuperInterface_Enter__(namespaceName, interfaceName) \
    struct interfaceName { \
        static const char* getInterfaceKey() { return #namespaceName"."#interfaceName; }
#define __SimpleWork_SuperInterface_Leave__ };

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
#include "SmartPtr.h"

#include "IFactory.h"
#include "IModule.h"
#include "ICoreModule.h"
#include "CoreApi.h"

//
// 类宏定义
//
#define SIMPLEWORK_CLASS_ENTER(x) \
class x : public SimpleWork::SmartPtr<I##x> { \
public: \
    x() : SmartPtr<I##x>() {} \
    x(const x& src) : SmartPtr<I##x>(src) {} \
    template<typename Q> x(const SmartPtr<Q>& src) : SmartPtr<I##x>(src) {};\
    template<typename Q> x(Q* pSrc) : SmartPtr<Q>(pSrc) {};
#define SIMPLEWORK_CLASS_LEAVE };

#include "Object.h"
#include "Factory.h"
#include "Module.h"
#include "CFactory.h"

#endif//__SimpleWork_Core_h__