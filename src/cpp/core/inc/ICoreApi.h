#ifndef __SimpleWork_ICoreApi__h__
#define __SimpleWork_ICoreApi__h__

__SimpleWork_Core_Namespace_Enter__

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACE_ENTER(ICoreApi, IObject, "sw.core.ICoreApi", 011130)

    //
    // 注册工厂
    //
    virtual int registerFactory(const char* szClassKey, IFactory* pFactory) = 0;

    //
    // 根据类名和接口名，创建对象
    // @szClassKey 类名，类似: "sw.math.Tensor", "sw.media.Video", "sw.io.Camera"
    //
    virtual IObjectPtr createObject(const char* szClassKey) = 0;

    //
    // 根据类名和接口名，创建工厂
    // @szClassKey 类名，类似:  "sw.math.Tensor", "sw.media.Video", "sw.io.Camera"
    //
    virtual IObjectPtr createFactory(const char* szClassKey) = 0;

    //
    // 如果不想实现自己的模块，则可以调用系统Api创建模块对象
    //
    virtual IModulePtr createModule(const char* szModuleKey = nullptr) = 0;

SIMPLEWORK_INTERFACE_LEAVE

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_ICoreApi__h__