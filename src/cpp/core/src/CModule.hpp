#ifndef __SimpleWork_CModule_H__
#define __SimpleWork_CModule_H__

#include "Core.h"

#include <map>
#include <string>
#include <iostream>

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #include "windows.h"
#endif

using namespace std;

__SimpleWork_Core_Namespace_Enter__

//
// 框架核心模块
//
//      sw框架通过唯一输出函数swcore::getCoreModule，来让其它模块获得系统
//  核心模块接口(IModule)，而实现就是这个模块。
//
class CModule : public CObject, public IModule {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IModule)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IModule
    int getSimpleWorkCompatibleVer() {
        return SIMPLEWORK_COMPATIBLE_VER;
    }

    //
    // 框架核心模块
    //
    //      sw框架通过唯一输出函数swcore::getCoreModule，来让其它模块获得系统
    //  核心模块接口(IModule)，而实现就是这个模块。
    //
    int initModule(const char* szModuleKey, ICoreApi* pCaller ) {
        if( pCaller ){
            //不允许重复初始化无效
            if( _spSuperModule ) {
                if( _spSuperModule.getPtr() == pCaller )
                return _spSuperModule.getPtr() == pCaller ? Error::Success : Error::Failure;
            }

            //不允许重命名（修改模块名）
            if(_strModuleKey.length() != 0 && _strModuleKey != string(szModuleKey)) {
                return Error::Failure;
            }

            //不允许模块名为空
            _strModuleKey = szModuleKey;
            if( _strModuleKey.length() == 0 ) {
                return Error::Failure;
            }

            //
            // 将已经注册的本地工厂，向上级模块注册，并删除本地注册，后续工作由上级模块带管
            //
            map<string, IFactoryPtr>::iterator it;
            for(it = _mapFactories.begin(); it != _mapFactories.end(); ++it) {
                string classKey = _strModuleKey + "." + it->first;
                pCaller->registerFactory(classKey.c_str(), it->second);
            }
        }

        _strModuleKey = szModuleKey;
        _spSuperModule = pCaller;
        _mapFactories.clear();
        return Error::Success;
    }

    IObjectPtr createObject(const char* szClassKey) {
        return _spSuperModule ? _spSuperModule->createObject(szClassKey) : nullptr;
    }

    //
    // 根据类名和接口名，创建工厂
    //
    IObjectPtr createFactory(const char* szClassKey) {
        return _spSuperModule ? _spSuperModule->createFactory(szClassKey) : nullptr;
    }

    //
    // 注册工厂
    //
    int registerFactory(const char* szClassKey, IFactory* pFactory) {
        if(_spSuperModule) {
            string strClassKey = _strModuleKey + "." + szClassKey;
            return _spSuperModule->registerFactory(strClassKey.c_str(), pFactory);
        }
        _mapFactories[szClassKey] = pFactory;
        return Error::Success;
    }

protected:
    string _strModuleKey;
    ICoreApiPtr _spSuperModule;
    map<string, IFactoryPtr> _mapFactories;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_CModule_H__