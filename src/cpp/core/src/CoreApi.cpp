#define SIMPLEWORK_WITHOUTAPI
#include "Core.h"
#include "CLibrary.hpp"

#include <map>
#include <string>
using namespace std;

__SimpleWork_Core_Namespace_Enter__

class CCoreApi : public CObject, public ICoreApi{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICoreApi)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    Object createObject(const char* szClassKey) {
        Factory spFactory = getRegisteredFactory(szClassKey);
        if( !spFactory ) {
            return Object();
        }
        return spFactory->createObject();
    }

    //
    // 根据类名和接口名，创建工厂
    //
    Object createFactory(const char* szClassKey) {
        return getRegisteredFactory(szClassKey);
    }

    int registerFactory(const char* szClassKey,  const Factory& pFactory) {
        m_mapFactories[szClassKey] = pFactory;
        return Error::Success;
    }

    Object createModule(const char* szModuleKey) {
        CoreApi coreapi = CoreApi::wrapPtr(this);
        Module spModule = CObject::createObject<CModule>();
        if( szModuleKey != nullptr && strlen(szModuleKey) > 0 ) {
            if( spModule->initModule(szModuleKey, coreapi) == Error::Success ) {
                m_mapSubModules[szModuleKey] = spModule;
            }
        }
        return spModule;
    }

    Factory getRegisteredFactory(const char* szClassKey) {
        map<string, Factory>::iterator it = m_mapFactories.find(szClassKey);
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
                return Factory();
            }

            //
            //  如果加载失败，则记录加载结果，下次无需再重复尝试加载
            //
            Module spModule = CLibrary::loadModule(strModuleKey);
            if( !spModule ) {
                //
                //  这个地方需要注意，如果反复尝试无效的名字，则有可能出现内存一直上
                //  涨的情况，也算是一种内存泄露，所以，为了避免这种情况，可以考虑把
                //  这一句去掉，副作用是，可能会反复尝试加载无法加载的模块.
                //
                m_mapSubModules[strModuleKey] = Module();
                return Factory();
            }

            m_mapSubModules[strModuleKey] = spModule;
            if(spModule->getSimpleWorkCompatibleVer() != SIMPLEWORK_COMPATIBLE_VER) {
                return Factory();
            }

            //
            // 初始化模块后，再查找一次工厂，看是否注册，并返回已经注册的值
            //
            CoreApi coreapi = CoreApi::wrapPtr(this);
            spModule->initModule(strModuleKey.c_str(), coreapi);
            it = m_mapFactories.find(szClassKey);
            if(it != m_mapFactories.end() ) {
                return it->second;
            }
        }
        return Factory();
    }

protected:
    map<string, Factory> m_mapFactories;
    map<string, Module> m_mapSubModules;
};

//
// 输出核心模块
//
__SimpleWork_API__ CoreApi& getCoreApi(int nCompatibleVer) {
    if(nCompatibleVer <= ICoreApi::getInterfaceVer() ) {
        static CoreApi s_spCoreApi = CObject::createObject<CCoreApi>();
        return s_spCoreApi;
    }else{
        static CoreApi s_coreapiNullpointer = CoreApi();
        return s_coreapiNullpointer;
    }
}

__SimpleWork_Core_Namespace_Leave__
