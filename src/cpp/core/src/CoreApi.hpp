#include "Core.h"
#include "CModule.hpp"
#include "CLibrary.hpp"

__SimpleWork_Core_Namespace_Enter__

class CCoreApi : public CObject, public ICoreApi{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICoreApi)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:
    IObjectPtr createObject(const char* szClassKey, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return nullptr;
        }
        IFactoryPtr spFactory = getRegisteredFactory(szClassKey);
        if( !spFactory ) {
            return nullptr;
        }
        return spFactory->createObject();
    }

    //
    // 根据类名和接口名，创建工厂
    //
    IObjectPtr createFactory(const char* szClassKey, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return nullptr;
        }
        return getRegisteredFactory(szClassKey);
    }

    int registerFactory(const char* szClassKey, IFactory* pFactory, int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return Error::Failure;
        }
        _mapFactories[szClassKey] = pFactory;
        return Error::Success;
    }

    IModulePtr createModule(int nSimpleWorkCompatibleVer) {
        if(nSimpleWorkCompatibleVer != SIMPLEWORK_COMPATIBLE_VER) {
            return nullptr;
        }
        return CFactory::createObject<CModule>();
    }

    IFactoryPtr getRegisteredFactory(const char* szClassKey) {
        map<string, IFactoryPtr>::iterator it = _mapFactories.find(szClassKey);
        if(it != _mapFactories.end() ) {
            return it->second;
        }

        string strClassKey = szClassKey;
        size_t iDotPos = strClassKey.rfind('.');
        if( iDotPos != string::npos && iDotPos > 0 ) {

            //
            //  如果模块已经加载，则无须继续加载
            //
            string strModuleKey = strClassKey.substr(0, iDotPos);
            if( _mapSubModules.find(strModuleKey) != _mapSubModules.end() ) {
                return nullptr;
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
                _mapSubModules[strModuleKey] = nullptr;
                return nullptr;
            }

            _mapSubModules[strModuleKey] = spModule;
            if(spModule->getSimpleWorkCompatibleVer() != SIMPLEWORK_COMPATIBLE_VER) {
                return nullptr;
            }

            //
            // 初始化模块后，再查找一次工厂，看是否注册，并返回已经注册的值
            //
            spModule->initModule(strModuleKey.c_str(), nullptr);
            it = _mapFactories.find(szClassKey);
            if(it != _mapFactories.end() ) {
                return it->second;
            }
        }
        return nullptr;
    }

public:
    static IModulePtr createSimpleWorkCoreModule() {
        ICoreApiPtr spCoreApi = getCoreApi();
        IModulePtr spCoreModule = CFactory::createObject<CModule>();
        spCoreModule->initModule("sw", spCoreApi);
        return spCoreModule;
    }

protected:
    IModulePtr _spSuperModule;
    map<string, IFactoryPtr> _mapFactories;
    map<string, IObjectPtr> _mapSubModules;
};

//
// 输出核心模块
//
__SimpleWork_API__ ICoreApiPtr getCoreApi(int nCompatibleVer) {
    static ICoreApiPtr s_spCoreApi = CFactory::createObject<CCoreApi>();
    return s_spCoreApi.getPtr();
}

__SimpleWork_Core_Namespace_Leave__
