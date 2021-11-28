#ifndef __SimpleWork_IModule__h__
#define __SimpleWork_IModule__h__

__SimpleWork_Core_Namespace_Enter__
    
struct ICoreApi;

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACE_ENTER(SIMPLEWORK_CORE_NAMESPACE, IModule, IObject)

    //
    // 获取系统版本号，如果返回的系统版本号，与系统不同，则会出现兼容问题。这个版本号，定义在全局宏
    //
    virtual int getSimpleWorkCompatibleVer() = 0;

    //
    // 初始化模块
    //
    virtual int initModule(const char* szModuleKey, ICoreApi* pCaller) = 0;

    //
    // 注册工厂
    //
    virtual int registerFactory(const char* szClassKey, IFactory* pFactory) = 0;

    //
    // 根据类名和接口名，创建对象
    // @szClassKey 类名，类似: "sw.Tensor", "sw.Video", "sw.Camera"
    //
    virtual IObjectPtr createObject(const char* szClassKey) = 0;

    //
    // 根据类名和接口名，创建工厂
    // @szClassKey 类名，类似: "sw.Tensor", "sw.Video", "sw.Camera"
    //
    virtual IObjectPtr createFactory(const char* szClassKey) = 0;

SIMPLEWORK_INTERFACE_LEAVE

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_IModule__h__