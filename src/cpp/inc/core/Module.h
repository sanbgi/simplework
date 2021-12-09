#ifndef __SimpleWork_IModule__h__
#define __SimpleWork_IModule__h__

#include "Object.h"

__SimpleWork_Core_Namespace_Enter__
class Module;
__SimpleWork_Core_Namespace_Leave__

#ifndef __SimpleWork_API__
    #ifndef SIMPLEWORK_WITHOUTAPI
        #if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
            #define __SimpleWork_API__ extern "C" __declspec(dllimport)
        #else
            #define __SimpleWork_API__ extern "C" 
        #endif
    #else//SIMPLEWORK_WITHOUTAPI
        #define __SimpleWork_API__ extern "C" 
    #endif//SIMPLEWORK_WITHOUTAPI
#endif//__SimpleWork_API__

//
//  获取当前模块
//
//      每一个模块都必须实现一个自己的__getSimpleWork函数，这是整个模块的核心全局函数。大
//  量的模块功能，都需要调用这个函数（调用的地方在各自头文件定义中）。并且系统加载模块时，也需要
//  能找到这个函数，进而获取模块指针，并初始化模块。
//
//      通过实现这个函数，模块甚至都不用依赖于连接SimpleWork库，因为SimpleWork库会自动加载模块，
//  并且会在初始化模块的时候，将系统模块指针作为参数，传递给当前模块，这样，当前模块就可以调用系统
//  模块功能了。
//      
//      当然，你可以直接使用宏SIMPLEWORK_MODULE_REGISTER(szModuleKey)，让系统自动帮你定义这个Api.
//
__SimpleWork_API__ SIMPLEWORK_CORE_NAMESPACE::Module& __getSimpleWork(int nCompatibleVer);

__SimpleWork_Core_Namespace_Enter__

//
// 模块对象
//
SIMPLEWORK_INTERFACECLASS_ENTER(Module)
    
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.IModule", 211202)

        //
        // 获取系统版本号，如果返回的系统版本号，与系统不同，则会出现兼容问题。这个版本号，定义在全局宏
        //
        virtual int getSimpleWorkCompatibleVer() = 0;

        //
        // 初始化模块
        //
        virtual int initModule(const char* szModuleKey, const Module& spCaller) = 0;

        //
        // 注册工厂
        //
        virtual int registerFactory(const char* szClassKey, const Factory& spFactory) = 0;

        //
        // 根据类名和接口名，创建对象
        // @szClassKey 类名，类似: "sw.Vector", "sw.Video", "sw.Camera"
        //
        virtual Object createObject(const char* szClassKey) = 0;

        //
        // 根据类名和接口名，创建工厂
        // @szClassKey 类名，类似: "sw.Vector", "sw.Video", "sw.Camera"
        //
        virtual Object createFactory(const char* szClassKey) = 0;

    SIMPLEWORK_INTERFACE_LEAVE


public:
    static inline Object createObject(const char* szClassKey) {
        return __getSimpleWork(IFace::getInterfaceVer())->createObject(szClassKey);
    }
    static inline Object createFactory(const char* szClassKey) {
        return __getSimpleWork(IFace::getInterfaceVer())->createFactory(szClassKey);
    }
    static inline Module& getSimpleWork() {
        return __getSimpleWork(IFace::getInterfaceVer());
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(Module)

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IModule__h__