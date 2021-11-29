#ifndef __SimpleWork_Core_Src_h__
#define __SimpleWork_Core_Src_h__

#if defined(_MSC_VER) || defined(_WIN32) || defined(_WIN64)
    #define __SimpleWork_API__ __declspec(dllexport)
#else
    #define __SimpleWork_API__
#endif

#include "../inc/Core.h"

#endif//__SimpleWork_Core_Src_h__