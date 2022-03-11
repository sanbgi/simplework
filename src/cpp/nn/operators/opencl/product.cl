typedef struct {
    int nIn;
    int nMat;
    int nOut;
}PProductParameter;

kernel void floatEval(
    global PProductParameter* pParameter, 
    int nBatchs,
    int nIn, global float * pIn,
    int nMat, global float * pMat,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    int iBatch = gid / pParameter->nOut;
    int iOut = gid % pParameter->nOut;
    global float *pItIn = pIn + (iBatch * pParameter->nIn) % nIn;
    global float *pItMat = pMat + (iBatch * pParameter->nMat) % nMat + iOut * pParameter->nIn;
    float fOut = 0.0f;
    nIn = pParameter->nIn;
    while(nIn-->0) {
        fOut += *pItIn * *pItMat;
        pItIn++, pItMat++;
    }
    pOut[gid] = fOut;
}

kernel void floatDevia(
    global PProductParameter* pParameter, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia,
    int nMat, global float * pMat, global float * pMatDevia,
    int nOut, global float * pOutDevia)
{
    int gid = get_global_id(0);
    int iBatch = gid / pParameter->nOut;
    int iOut = gid % pParameter->nOut;
    int iInOffset = (iBatch * pParameter->nIn) % nIn;
    int iMatOffset = (iBatch * pParameter->nMat) % nMat + iOut * pParameter->nIn;
    global float *pItIn = pIn + iInOffset;
    global float *pItInDevia = pInDevia + iInOffset;
    global float *pItMat = pMat + iMatOffset;
    global float *pItMatDevia = pMatDevia + iMatOffset;
    nIn = pParameter->nIn;
    
    float fOutDevia = pOutDevia[gid];
    union {
        unsigned int intVal;
        float floatVal;
    } newVal, prevVal;
    while(nIn-->0) {
        do {
            prevVal.floatVal = *pItInDevia;
            newVal.floatVal = prevVal.floatVal + fOutDevia * (*pItMat);
        } while (atomic_cmpxchg((volatile __global unsigned int *)pItInDevia, 
                                prevVal.intVal, newVal.intVal) 
                                != prevVal.intVal);

        do {
            prevVal.floatVal = *pItMatDevia;
            newVal.floatVal = prevVal.floatVal + fOutDevia * (*pItIn);
        } while (atomic_cmpxchg((volatile __global unsigned int *)pItMatDevia, 
                                prevVal.intVal, newVal.intVal) 
                                != prevVal.intVal);
        pItIn++, pItMat++, pItInDevia++, pItMatDevia++;
    }
    
    /*
    // Have a child kernel write into third quarter of output
    ndrange_t ndrange = ndrange_1D(nIn);
    enqueue_kernel(get_default_queue(), CLK_ENQUEUE_FLAGS_WAIT_KERNEL, ndrange,
    ^{
        int i=get_global_id(0);
        pItInDevia[i] += pItMat[i]*fOutDevia;
        pItMatDevia[i] += pIn[i]*fOutDevia; 
    });
    */
}
