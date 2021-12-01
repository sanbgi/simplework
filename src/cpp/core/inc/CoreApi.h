#ifndef __SimpleWork_CoreApi_h__
#define __SimpleWork_CoreApi_h__

#ifndef SIMPLEWORK_MODULE_EXPORT
    #if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
        #define SIMPLEWORK_MODULE_EXPORT extern "C" __declspec(dllexport)
    #else
        #define SIMPLEWORK_MODULE_EXPORT extern "C" 
    #endif
#endif//SIMPLEWORK_MODULE_EXPORT

//
//  获取当前模块
//
//      每一个模块都必须实现一个自己的getSimpleWork函数，这是整个模块的核心全局函数。大
//  量的模块功能，都需要调用这个函数（调用的地方在各自头文件定义中）。并且系统加载模块时，也需要
//  能找到这个函数，进而获取模块指针，并初始化模块。
//
//      通过实现这个函数，模块甚至都不用依赖于连接SimpleWork库，因为SimpleWork库会自动加载模块，
//  并且会在初始化模块的时候，将系统模块指针作为参数，传递给当前模块，这样，当前模块就可以调用系统
//  模块功能了。
//
SIMPLEWORK_MODULE_EXPORT SIMPLEWORK_CORE_NAMESPACE::IModule* getSimpleWork();

#ifndef SIMPLEWORK_WITHOUTAPI

__SimpleWork_Core_Namespace_Enter__

//
// 获取系统核心Api接口
//
// @nCompatibleVer
//      当前库的版本号
//
// @remark
//
//      这框架唯一一个输出函数，其它模块可以通过链接这个函数，获取框架相关能力。
//
__SimpleWork_API__ ICoreApi* getCoreApi(int nCompatibleVer=ICoreApi::getInterfaceVer());

__SimpleWork_Core_Namespace_Leave__

#endif//SIMPLEWORK_WITHOUTAPI

#endif//__SimpleWork_CoreApi_h__