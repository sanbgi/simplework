kernel void floatEval(
    global void* pThis, 
    int nBatchs,
    int nIn, global float * pIn,
    int nOut, global float * pOut)
{
    int nPoolSize = nIn / nOut;
    int nOutTensorSize = nOut / nBatchs;
    int nInTensorSize = nOutTensorSize * nPoolSize;

    int gid = get_global_id(0);
    int iBatch = gid / nOutTensorSize;
    int iOut = gid % nOutTensorSize;
    pIn += nInTensorSize * iBatch + iOut;

    float sumPlane = 0.0f;
    for(int i=0; i<nPoolSize; i++) {
        sumPlane += *pIn;
        pIn += nOutTensorSize;
    }
    pOut[gid] = sumPlane / nPoolSize;
}

kernel void floatDevia(
    global void* pThis, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia,
    int nOut, global float * pOutDevia)
{
    int nPoolSize = nIn / nOut;
    int nOutTensorSize = nOut / nBatchs;
    int nInTensorSize = nOutTensorSize * nPoolSize;

    int gid = get_global_id(0);
    int iBatch = gid / nOutTensorSize;
    int iOut = gid % nOutTensorSize;
    pInDevia += nInTensorSize * iBatch + iOut;
    float deviaO = pOutDevia[gid] / nPoolSize;
    for(int i=0; i<nPoolSize; i++) {
        *pInDevia += deviaO;
        pInDevia += nOutTensorSize;
    }
}
