#ifndef __SimpleWork_nn_nn_h__
#define __SimpleWork_nn_nn_h__

#include "../inc/nn/nn.h"
#include "../inc/io/io.h"
#include "../inc/device/device.h"

#ifndef _DEBUG
#define _DEBUG
#endif//_DEBUG

#ifdef _DEBUG

    #ifndef VERIFY
    #define VERIFY(x) if(!(x)) { *((double*)nullptr) = 0; }
    #endif

#else//_DEBUG

    #ifndef VERIFY
    #define VERIFY(x) 
    #endif

#endif//

#endif//__SimpleWork_nn_nn_h__