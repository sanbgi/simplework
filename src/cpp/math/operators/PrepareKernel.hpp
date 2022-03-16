#ifndef __SimpleWork_Math_Operators_PrepareKernel_H__
#define __SimpleWork_Math_Operators_PrepareKernel_H__

#define global
#define __global
#define volatile
#define kernel
#define WITHOUT_OPENCL
#define atomplus(ptr,v) *(ptr) += v

static int get_global_offset(int) {
    return 0;
}

int get_global_id(int i) {
    return pCtx->pRanges[i];
}

#endif//__SimpleWork_Math_Operators_PrepareKernel_H__