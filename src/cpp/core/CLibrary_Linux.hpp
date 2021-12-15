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

SIMPLEWORK_INTERFACECLASS_ENTER0(Library)
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ILibrary", 011130)
        virtual SModule loadLibraryModule(string strModuleKey) = 0;
    SIMPLEWORK_INTERFACE_LEAVE
SIMPLEWORK_INTERFACECLASS_LEAVE(Library)


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
        { return m_spModule ? m_spModule->getSimpleWorkCompatibleVer() : IModule::getInterfaceVer();}
    int initModule(const char* szModuleKey, const SModule& pCaller) 
        { return m_spModule ? m_spModule->initModule(szModuleKey, pCaller) : SError::ERRORTYPE_SUCCESS; }
    int registerFactory(const char* szClassKey, const SFactory& pFactory) 
        { return SError::ERRORTYPE_FAILURE; }
    int createObject(const char* szClassKey, SObject& rObject) 
        { return SError::ERRORTYPE_FAILURE; }
    static SModule loadModule(string strModuleKey) {
        SLibrary spLibrary = CObject::createObject<CLibrary>();
        return spLibrary->loadLibraryModule(strModuleKey);
    }
    
private:
    void* _pDLL;
    SModule m_spModule;

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
    SModule loadLibraryModule(string strModuleKey) {
        //
        // 释放已经加载的模块
        //
        if( _pDLL != nullptr ) {
            m_spModule = Object();
            dlclose(_pDLL);
            _pDLL = nullptr;
        }

        string strLibrary = "lib"+strModuleKey+".so";
        void *lib_dl = dlopen(strLibrary.c_str(),RTLD_LAZY);
        if (lib_dl)
        {
            _pDLL = lib_dl;
            typedef SIMPLEWORK_CORE_NAMESPACE::SModule& (*FUNCTION)();
            FUNCTION fun = (FUNCTION)dlsym(lib_dl,"getSimpleWork");
            if (fun)
            { 
                m_spModule = (*fun)();
            }
            return SModule::wrapPtr(this);
        }

        return SObject();
    }
};

__SimpleWork_Core_Namespace_Leave__
#endif//__SimpleWork_CLibrary_H__
#endif//#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
