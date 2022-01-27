#ifndef __SimpleWork_CModule_H__
#define __SimpleWork_CModule_H__
#ifdef SIMPLEWORK_WITHOUTAPI

#include <map>
#include <string>
#include "core.h"

__SimpleWork_Core_Namespace_Enter__

//
//  模块类：帮助实现全局模块对象。
//
//      不建议直接使用，而是通过宏SIMPLEWORK_MODULE_REGISTER间接使用这个类。这个类只有在模块
//  没有定义宏SIMPLEWORK_MODULE_REGISTER同时定义了宏SIMPLEWORK_WITHOUTAPI的情况下才会被引
//  入，以达到最小化影响全局头文件的目的。
//      
//      由于模块依赖于<map><string>，所以，能不引入则尽量不要引入。
//
class CModule : public CObject, public IModule {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IModule)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

protected://IModule
    //
    // 获取兼容版本号，系统会率先调用这个函数，检查版本号，符合要求，才会继续使用模块
    //
    int getSimpleWorkCompatibleVer() {
        return IModule::getInterfaceVer();
    }

    //
    // 框架核心模块
    //
    //      sw框架通过唯一输出函数sw::getCoreModule，来让其它模块获得系统
    //  核心模块接口(IModule)，而实现就是这个模块。
    //
    int initModule(const char* szModuleKey, const SModule& pCaller ) {
        if( pCaller ){
            //不允许重复初始化无效
            if( m_spCoreApi ) {
                return m_spCoreApi == pCaller ? SError::ERRORTYPE_SUCCESS : SError::ERRORTYPE_FAILURE;
            }

            //
            // 将已经注册的本地工厂，向上级模块注册，并删除本地注册，后续工作由上级模块带管
            //
            std::map<std::string, SFactory>::iterator it;
            for(it = m_mapFactories.begin(); it != m_mapFactories.end(); ++it) {
                pCaller->registerFactory(it->first.c_str(), it->second);
            }
        }

        m_strModuleKey = szModuleKey;
        m_spCoreApi = pCaller;
        m_mapFactories.clear();
        return SError::ERRORTYPE_SUCCESS;
    }

    //
    // 根据对象实现类名，创建对象
    //
    int createObject(const char* szClassKey, const PData* pData, SObject& rObject) {
        if(m_spCoreApi) {
            return m_spCoreApi->createObject(szClassKey, pData, rObject);
        }
        IFactory* pFactory = findFactory(szClassKey);
        if(pFactory == nullptr) {
            return SError::ERRORTYPE_FAILURE;
        }
        return pFactory->createObject(rObject, pData);
    }

    //
    // 注册工厂
    //
    int registerFactory(const char* szClassKey, const SFactory& pFactory) {
        if(m_spCoreApi) {
            return m_spCoreApi->registerFactory(szClassKey, pFactory);
        }
        m_mapFactories[szClassKey] = pFactory;
        return SError::ERRORTYPE_SUCCESS;
    }

protected:
    IFactory* findFactory(const char* szClassKey) {
        std::map<std::string,SFactory>::iterator it = m_mapFactories.find(szClassKey);
        return (it != m_mapFactories.end()) ? it->second.getPtr() : nullptr;
    }

protected:
    SModule m_spCoreApi;
    std::string m_strModuleKey;
    std::map<std::string, SFactory> m_mapFactories;
};

__SimpleWork_Core_Namespace_Leave__

#endif//SIMPLEWORK_WITHOUTAPI
#endif//__SimpleWork_CModule_H__