#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
#ifndef __SimpleWork_CLibrary_H__
#define __SimpleWork_CLibrary_H__

#include "windows.h"

__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_INTERFACE_ENTER(SIMPLEWORK_CORE_NAMESPACE, ILibrary, IObject)
    virtual IModulePtr loadLibraryModule(string strModuleKey) = 0;
SIMPLEWORK_INTERFACE_LEAVE

//
// 框架核心模块
//
//      sw框架通过唯一输出函数swcore::getCoreModule，来让其它模块获得系统
//  核心模块接口(IModule)，而实现就是这个模块。
//
class CLibrary : public CObject, ILibrary, IModule {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ILibrary)
        SIMPLEWORK_INTERFACE_ENTRY(IModule)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int getSimpleWorkCompatibleVer() 
        { return _spModule->getSimpleWorkCompatibleVer();}
    int initModule(const char* szModuleKey, ICoreApi* pCaller) 
        { return _spModule->initModule(szModuleKey, pCaller); }
    int registerFactory(const char* szClassKey, IFactory* pFactory) 
        { return Error::Failure; }
    IObjectPtr createObject(const char* szClassKey) 
        { return nullptr; }
    IObjectPtr createFactory(const char* szClassKey)
        { return nullptr; }
    static IModulePtr loadModule(string strModuleKey) {
        ILibraryPtr spLibrary = CFactory::createObject<CLibrary>();
        return spLibrary->loadLibraryModule(strModuleKey);
    }
    
private:
    HMODULE _hDLL;
    IModulePtr _spModule;

public:
    CLibrary() {
        _hDLL = NULL;
    }
    ~CLibrary() {
        if( _hDLL != NULL ) {
            FreeLibrary(_hDLL);
            _hDLL = NULL;
        }
    }

private:
    IModulePtr loadLibraryModule(string strModuleKey) {
        //
        // 释放已经加载的模块
        //
        if( _hDLL != NULL ) {
            _spModule = nullptr;
            FreeLibrary(_hDLL);
            _hDLL = NULL;
        }

        string strLibrary = strModuleKey+".dll";
        HMODULE hDLL = LoadLibrary(strLibrary.c_str());
        if (hDLL)
        {
            _hDLL = hDLL;

            typedef SIMPLEWORK_CORE_NAMESPACE::IModule* (*FUNCTION)();
            FUNCTION fun = (FUNCTION)GetProcAddress(hDLL, "getSimpleWorkModule");
            if (fun)
            { 
                _spModule = (*fun)();
                return (IModule*)this;
            }
        }

        return nullptr;
    }
};

__SimpleWork_Core_Namespace_Leave__
#endif//__SimpleWork_CLibrary_H__
#endif//defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
