typedef struct {
}PPlusParameter;

kernel void plus_eval(
    global PPlusParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn1,
    int nIn2, global float * pIn2,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn1[gid%nIn1] + pIn2[gid%nIn2];
}

kernel void plus_devia(
    global PPlusParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn1, global float * pIn1Devia,
    int nIn2, global float * pIn2, global float * pIn2Devia,
    int nOut, global float * pOutDevia)
{
    int gid = get_global_id(0);
    union {
        unsigned int intVal;
        float floatVal;
    } newVal, prevVal;

    pIn1Devia += gid%nIn1;
    do {
        prevVal.floatVal = (*pIn1Devia);
        newVal.floatVal = prevVal.floatVal + pOutDevia[gid];
    } while (atomic_cmpxchg((volatile __global unsigned int *)pIn1Devia, 
                            prevVal.intVal, newVal.intVal) 
                            != prevVal.intVal);

    pIn2Devia += gid%nIn2;
    do {
        prevVal.floatVal = (*pIn2Devia);
        newVal.floatVal = prevVal.floatVal + pOutDevia[gid];
    } while (atomic_cmpxchg((volatile __global unsigned int *)pIn2Devia, 
                            prevVal.intVal, newVal.intVal) 
                            != prevVal.intVal);
    //pIn1Devia[gid%nIn1] += pOutDevia[gid];
    //pIn2Devia[gid%nIn2] += pOutDevia[gid];
}
