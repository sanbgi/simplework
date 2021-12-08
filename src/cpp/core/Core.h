#ifndef __SimpleWork_Core_Src_h__
#define __SimpleWork_Core_Src_h__

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #define __SimpleWork_API__ extern "C" __declspec(dllexport)
#else
    #define __SimpleWork_API__
#endif

#include "../inc/core/Core.h"

#endif//__SimpleWork_Core_Src_h__