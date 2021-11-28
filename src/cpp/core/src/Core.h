#ifndef __SimpleWork_Core_Src_h__
#define __SimpleWork_Core_Src_h__

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #define __SimpleWork_API__ __declspec(dllexport)
#else
    #define __SimpleWork_API__
#endif

#define getSimpleWorkModule __getSimpleWorkCoreModule
#define __SimpleWork_getSimpleWorkModule__ SIMPLEWORK_CORE_NAMESPACE::IModule* __getSimpleWorkCoreModule();

#include "../inc/Core.h"

#endif//__SimpleWork_Core_Src_h__