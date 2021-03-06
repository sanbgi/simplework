
#include "Core.h"
#include "CLibrary.hpp"
#include "CCtx.h"

#include <map>
#include <string>
using namespace std;

__SimpleWork_Core_Namespace_Enter__

static SCtx sCtx = CCtx::createCtx("CModule");
class CModule : public CObject, public IModule{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IModule)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private:

    int getSimpleWorkCompatibleVer() {
        return IModule::getInterfaceVer();
    }

    int initModule(const char* szModuleKey, const SModule& pCaller) {
        return sCtx.success();
    }

    int createObject(const char* szClassKey, const PData* pData, SObject& spObject) {
        const SFactory& spFactory = getRegisteredFactory(szClassKey);
        if( !spFactory ) {
            return sCtx.error();
        }
        return spFactory->createObject(spObject, pData);
    }

    //
    // 根据类名和接口名，创建工厂
    //
    //Object createFactory(const char* szClassKey) {
    //    return getRegisteredFactory(szClassKey);
    //}

    int registerFactory(const char* szClassKey,  const SFactory& pFactory) {
        m_mapFactories[szClassKey] = pFactory;
        return sCtx.success();
    }

    const SFactory& getRegisteredFactory(const char* szClassKey) {
        map<string, SFactory>::iterator it = m_mapFactories.find(szClassKey);
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
                return m_spNullfactory;
            }

            //
            //  如果加载失败，则记录加载结果，下次无需再重复尝试加载
            //
            SModule spModule = CLibrary::loadModule(strModuleKey);
            if( !spModule ) {
                //
                //  这个地方需要注意，如果反复尝试无效的名字，则有可能出现内存一直上
                //  涨的情况，也算是一种内存泄露，所以，为了避免这种情况，可以考虑把
                //  这一句去掉，副作用是，可能会反复尝试加载无法加载的模块.
                //
                m_mapSubModules[strModuleKey] = SModule();
                return m_spNullfactory;
            }

            m_mapSubModules[strModuleKey] = spModule;
            if(spModule->getSimpleWorkCompatibleVer() < IModule::getInterfaceVer()) 
            {
                return m_spNullfactory;
            }

            //
            // 初始化模块后，再查找一次工厂，看是否注册，并返回已经注册的值
            //
            SModule coreapi = SModule::wrapPtr((IModule*)this);
            spModule->initModule(strModuleKey.c_str(), coreapi);
            it = m_mapFactories.find(szClassKey);
            if(it != m_mapFactories.end() ) {
                return it->second;
            }
        }
        return m_spNullfactory;
    }

protected:
    map<string, SFactory> m_mapFactories;
    map<string, SModule> m_mapSubModules;
    SFactory m_spNullfactory;
};

//
// 输出核心模块
//
__SimpleWork_API__ SModule& __getSimpleWork(int nCompatibleVer) {
    if(nCompatibleVer <= IModule::getInterfaceVer() ) {
        static SModule s_spCoreModule = CObject::createObject<CModule>();
        return s_spCoreModule;
    }else{
        static SModule s_coreapiNullpointer = SModule();
        return s_coreapiNullpointer;
    }
}

__SimpleWork_Core_Namespace_Leave__
