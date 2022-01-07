#ifndef __SimpleWork_nn_nn_h__
#define __SimpleWork_nn_nn_h__

#include "../inc/nn/nn.h"

#ifndef _DEBUG
#define _DEBUG
#endif//_DEBUG

#ifdef _DEBUG
#define CHECK_POINTER
#endif//

#ifdef CHECK_POINTER
    struct CNullV {
        template<typename Q> static Q& getNullV() {
            Q* pQ = nullptr;
            return *pQ;
        }
    };
    #define DVV(ptr, index, size) ((index >= 0 && index < size) ? ptr[index] : CNullV::getNullV<double>()) 
    #define IVV(ptr, index, size) ((index >= 0 && index < size) ? ptr[index] : CNullV::getNullV<int>()) 
#else//
    #define DVV(ptr, index, size) ptr[index] 
    #define IVV(ptr, index, size) ptr[index] 
#endif//CHECK_POINTER

#endif//__SimpleWork_nn_nn_h__