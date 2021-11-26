#ifndef __SimpleWork_IModule__h__
#define __SimpleWork_IModule__h__

namespace SimpleWork {

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACE_ENTER(SimpleWork, IModule, IObject)

    //
    // 获取系统版本号
    // 
    // @remark
    //      如果返回的系统版本号，与系统不同，则会出现兼容问题。这个版本号，定义在全局宏
    //
    virtual int getSimpleWorkVer() = 0;

    //
    // 初始化模块
    //
    virtual int initModule(IModule* pCaller) = 0;

    //
    // 创建对象
    //
    // @szClassKey 对象标识，例如："SimpleWork.Tensor"      
    //
    SmartPtr<IObject> createObject(const char* szClassKey) {
        SmartPtr<IObject> spObject;
        createObject(nullptr, szClassKey, &spObject);
        return spObject;
    }

    //
    // 创建工厂
    //
    // @szClassKey 对象标识，例如："SimpleWork.Tensor"      
    //
    SmartPtr<IObject> createFactory(const char* szClassKey) {
        SmartPtr<IObject> spFactory;
        createFactory(nullptr, szClassKey, &spFactory);
        return spFactory;
    }

    //
    // 注册工厂
    //
    virtual int registerFactory(IModule* pCaller, const char* szClassKey, IFactory* pFactory) = 0;


protected:
    //
    // 根据类名和接口名，创建对象
    //
    // @szClassKey 类名，类似: "SimpleWork.Tensor", "SimpleWork.Video", "SimpleWork.Camera"
    // @szInterfaceKey 接口名，类似："SimpleWork.ITensor"
    //
    virtual int createObject(IModule* pCaller, const char* szClassKey, IObjectContainer* pContainer) = 0;

    //
    // 根据类名和接口名，创建工厂
    //
    virtual int createFactory(IModule* pCaller, const char* szClassKey, IObjectContainer* pContainer) = 0;

SIMPLEWORK_INTERFACE_LEAVE

}//namespace SimpleWork

#endif//__SimpleWork_IModule__h__