#ifndef WITHOUT_OPENCL

    void atomplus(global float* p, float v) {
        union {
            unsigned int intVal;
            float floatVal;
        } newVal, prevVal;
        do {
            prevVal.floatVal = *p;
            newVal.floatVal = prevVal.floatVal + v;
        } while (atomic_cmpxchg((volatile __global unsigned int *)p, 
                                prevVal.intVal, newVal.intVal) 
                                != prevVal.intVal);
    }

#endif//WITHOUT_OPENCL

typedef struct {
}PPlusParameter;

kernel void floatEval(
    global PPlusParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn1,
    int nIn2, global float * pIn2,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn1[gid%nIn1] + pIn2[gid%nIn2];
}

kernel void floatDevia(
    global PPlusParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn1, global float * pIn1Devia,
    int nIn2, global float * pIn2, global float * pIn2Devia,
    int nOut, global float * pOutDevia)
{
    int gid = get_global_id(0);
    atomplus(pIn1Devia+gid%nIn1, pOutDevia[gid]);
    atomplus(pIn2Devia+gid%nIn2, pOutDevia[gid]);
}
