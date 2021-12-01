#if defined(_UNIX) || defined(linux) 

#ifndef __SimpleWork_CLibrary_H__
#define __SimpleWork_CLibrary_H__

#include <map>
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <dlfcn.h>
#include <iostream>
using namespace std;

__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_INTERFACE_ENTER(ILibrary, IObject, "sw.core.ILibrary", 211201)
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
    int initModule(const char* szModuleKey, ICoreApi* pCaller ) 
        { return _spModule->initModule(szModuleKey, pCaller); }
    int registerFactory(const char* szClassKey, IFactory* pFactory) 
        { return Error::Failure; }
    IObjectPtr createObject(const char* szClassKey) 
        { return IObjectNullptr; }
    IObjectPtr createFactory(const char* szClassKey)
        { return IObjectNullptr; }
    static IModulePtr loadModule(string strModuleKey) {
        ILibraryPtr spLibrary = CObject::createObject<CLibrary>();
        return spLibrary->loadLibraryModule(strModuleKey);
    }
    
private:
    void* _pDLL;
    IModulePtr _spModule;

public:
    CLibrary() {
        _pDLL = nullptr;
    }
    ~CLibrary() {
        if( _pDLL != nullptr ) {
            dlclose(_pDLL);
            _pDLL = nullptr;
        }
    }

private:
    IModulePtr loadLibraryModule(string strModuleKey) {
        //
        // 释放已经加载的模块
        //
        if( _pDLL != nullptr ) {
            _spModule = IObjectNullptr;
            dlclose(_pDLL);
            _pDLL = nullptr;
        }

        string strLibrary = "lib"+strModuleKey+".so";
        void *lib_dl = dlopen(strLibrary.c_str(),RTLD_LAZY);
        if (lib_dl)
        {
            _pDLL = lib_dl;

            typedef IModule* (*FUNCTION)();
            FUNCTION fun = (FUNCTION)dlsym(lib_dl,"getSimpleWork");
            if (fun)
            { 
                _spModule = (*fun)();
                return (IModule*)this;
            }
        }

        return IObjectNullptr;
    }
};

__SimpleWork_Core_Namespace_Leave__
#endif//__SimpleWork_CLibrary_H__
#endif//#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
