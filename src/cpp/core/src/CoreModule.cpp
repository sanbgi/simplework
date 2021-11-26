#define __SimpleWork_Module__
#include "../inc/Core.h"

#include <map>
#include <string>
#include <iostream>

namespace SimpleWork {

//
// 框架核心模块
//
// @remark
//  
//      SimpleWork框架通过唯一输出函数swcore::getCoreModule，来让其它模块获得系统
//  核心模块接口(IModule)，而实现就是这个模块。
//
class CoreModule : public CObject, ICoreModule {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICoreModule)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IModule
    int getSimpleWorkVer() {
        return SIMPLEWORK_COMPATIBLE_VER;
    }

    int initModule(IModule* pCaller) {
        if( pCaller ) {
            _spSuperModule = pCaller;
        }
        return Error::Success;
    }

    IObjectPtr createObject(const char* szClassKey, IModule* pCaller) {
        SmartPtr<IFactory> spFactory = getRegisteredFactory(szClassKey);
        if( !spFactory ) {
            return spFactory->createObject();
        }
        return IObjectPtr();
    }

    //
    // 根据类名和接口名，创建工厂
    //
    IObjectPtr createFactory(const char* szClassKey, IModule* pCaller) {
        return getRegisteredFactory(szClassKey);
    }

    //
    // 注册工厂
    //
    int registerFactory(const char* szClassKey, IFactory* pFactory, IModule* pCaller) {
        std::cout << "classKey = " << szClassKey;
        _mapFactories[szClassKey] = SmartPtr<IFactory>(pFactory);
        return Error::Success;
    }

public://ICoreModule
    IObjectPtr createModule(const char* szModuleKey) {
        return IObjectPtr();
    }

public://Constructor
    CoreModule() {
    }

    ~CoreModule() {
    }

private:
    SmartPtr<IModule> _spSuperModule;
    std::map<std::string, SmartPtr<IFactory>> _mapFactories;

private:
    SmartPtr<IFactory> getRegisteredFactory(const char* szClassKey) {
        std::map<std::string, SmartPtr<IFactory>>::iterator it = _mapFactories.find(szClassKey);
        if(it != _mapFactories.end() ) {
            return it->second;
        }
        return SmartPtr<IFactory>();
    }
};

//
// 输出核心模块
//
__SimpleWork_API__ ICoreModule* getCoreModule(int nCompatibleVer) {
    static SmartPtr<ICoreModule> s_spCoreModule = CFactory::createObject<CoreModule>();
    return s_spCoreModule.getPtr();
}

}//namespace SimpleWork

//
// 输出核心模块内部使用的全局模块，供模块内部使用
//
SimpleWork::IModule* getSimpleWorkModule() {
    return SimpleWork::getCoreModule();
}