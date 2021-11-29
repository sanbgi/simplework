#define SIMPLEWORK_WITHOUTAPI
#include "Core.h"
#include "CLibrary.hpp"

#include <map>
#include <string>
#include "windows.h"
using namespace std;

__SimpleWork_Core_Namespace_Enter__

class CCoreApi : public CObject, public ICoreApi{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICoreApi)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    IObjectPtr createObject(const char* szClassKey, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return IObjectNullptr;
        }
        IFactoryPtr spFactory = getRegisteredFactory(szClassKey);
        if( !spFactory ) {
            return IObjectNullptr;
        }
        return spFactory->createObject();
    }

    //
    // 根据类名和接口名，创建工厂
    //
    IObjectPtr createFactory(const char* szClassKey, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return IObjectNullptr;
        }
        return getRegisteredFactory(szClassKey);
    }

    int registerFactory(const char* szClassKey, IFactory* pFactory, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return Error::Failure;
        }
        m_mapFactories[szClassKey] = pFactory;
        return Error::Success;
    }

    IModulePtr createModule(const char* szModuleKey, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return IObjectNullptr;
        }

        IModulePtr spModule = CFactory::createObject<CModule>();
        if( szModuleKey != nullptr && strlen(szModuleKey) > 0 ) {
            if( spModule->initModule(szModuleKey, this) == Error::Success ) {
                m_mapSubModules[szModuleKey] = spModule;
            }
        }
        return spModule;
    }

    IFactoryPtr getRegisteredFactory(const char* szClassKey) {
        map<string, IFactoryPtr>::iterator it = m_mapFactories.find(szClassKey);
        if(it != m_mapFactories.end() ) {
            return it->second;
        }

        string strClassKey = szClassKey;
        size_t iDotPos = strClassKey.rfind('.');
        if( iDotPos != string::npos && iDotPos > 0 ) {

            //
            //  如果模块已经加载，则无须继续加载
            //
            string strModuleKey = strClassKey.substr(0, iDotPos);
            if( m_mapSubModules.find(strModuleKey) != m_mapSubModules.end() ) {
                return IObjectNullptr;
            }

            //
            //  如果加载失败，则记录加载结果，下次无需再重复尝试加载
            //
            IModulePtr spModule = CLibrary::loadModule(strModuleKey);
            if( !spModule ) {
                //
                //  这个地方需要注意，如果反复尝试无效的名字，则有可能出现内存一直上
                //  涨的情况，也算是一种内存泄露，所以，为了避免这种情况，可以考虑把
                //  这一句去掉，副作用是，可能会反复尝试加载无法加载的模块.
                //
                m_mapSubModules[strModuleKey] = IObjectNullptr;
                return IObjectNullptr;
            }

            m_mapSubModules[strModuleKey] = spModule;
            if(spModule->getSimpleWorkCompatibleVer() != SIMPLEWORK_COMPATIBLE_VER) {
                return IObjectNullptr;
            }

            //
            // 初始化模块后，再查找一次工厂，看是否注册，并返回已经注册的值
            //
            spModule->initModule(strModuleKey.c_str(), this);
            it = m_mapFactories.find(szClassKey);
            if(it != m_mapFactories.end() ) {
                return it->second;
            }
        }
        return IObjectNullptr;
    }

protected:
    map<string, IFactoryPtr> m_mapFactories;
    map<string, IObjectPtr> m_mapSubModules;
};

//
// 输出核心模块
//
__SimpleWork_API__ ICoreApi* getCoreApi(int nCompatibleVer) {
    static ICoreApiPtr s_spCoreApi = CFactory::createObject<CCoreApi>();
    return s_spCoreApi;
}

__SimpleWork_Core_Namespace_Leave__
