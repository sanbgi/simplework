#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
#ifndef __SimpleWork_CLibrary_H__
#define __SimpleWork_CLibrary_H__

#include <string>
#include "windows.h"
using namespace std;

__SimpleWork_Core_Namespace_Enter__

SIMPLEWORK_INTERFACECLASS_ENTER0(Library)
    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.core.ILibrary", 011130)
        virtual Module loadLibraryModule(string strModuleKey) = 0;
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
        { return _spModule ? _spModule->getSimpleWorkCompatibleVer() : IModule::getInterfaceVer();}
    int initModule(const char* szModuleKey, const Module& pCaller) 
        { return _spModule ? _spModule->initModule(szModuleKey, pCaller) : Error::ERRORTYPE_SUCCESS; }
    int registerFactory(const char* szClassKey, const Factory& pFactory) 
        { return Error::ERRORTYPE_FAILURE; }
    Object createObject(const char* szClassKey) 
        { return Object(); }
    Object createFactory(const char* szClassKey)
        { return Object(); }
    static Module loadModule(string strModuleKey) {
        Library spLibrary = CObject::createObject<CLibrary>();
        return spLibrary->loadLibraryModule(strModuleKey);
    }
    
private:
    HMODULE _hDLL;
    Module _spModule;

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
    Module loadLibraryModule(string strModuleKey) {
        //
        // 释放已经加载的模块
        //
        if( _hDLL != NULL ) {
            _spModule = Module();
            FreeLibrary(_hDLL);
            _hDLL = NULL;
        }

        string strLibrary = "lib"+strModuleKey+".dll";
        HMODULE hDLL = LoadLibrary(strLibrary.c_str());
        if (hDLL)
        {
            _hDLL = hDLL;

            typedef SIMPLEWORK_CORE_NAMESPACE::Module& (*FUNCTION)();
            FUNCTION fun = (FUNCTION)GetProcAddress(hDLL, "getSimpleWork");
            if (fun)
            { 
                _spModule = (*fun)();
            }
            return Module::wrapPtr(this);
        }

        return Module();
    }
};

__SimpleWork_Core_Namespace_Leave__
#endif//__SimpleWork_CLibrary_H__
#endif//defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
