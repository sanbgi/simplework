typedef struct {
}PSoftmaxParameter;

kernel void floatEval(
    global PSoftmaxParameter* pParameter, 
    int nBatchs,
    int nIn, global float * pIn,
    int nOut, global float * pOut)
{
    int iBatch = get_global_id(0);
    int nSize = nOut / nBatchs;
    pIn = pIn + iBatch * nSize;
    pOut = pOut + iBatch * nSize;

    float dMax = pIn[0];
    for(int i=1; i<nSize; i++ ){
        if(pIn[i]>dMax) {
            dMax = pIn[i];
        }
    }

    float dSum = 0;
    for(int i=0; i<nSize; i++) {
        pOut[i] = (float)exp(pIn[i]-dMax);
        dSum += pOut[i];
    }

    for(int i=0; i<nSize; i++) {
        pOut[i] = pOut[i]/dSum;
    }
}

kernel void floatDevia(
    global PSoftmaxParameter* pParameter, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia, 
    int nOut, global float * pOutDevia)
{
    int iBatch = get_global_id(0);
    int nSize = nOut / nBatchs;
    pInDevia = pInDevia + iBatch * nSize;
    pOutDevia = pOutDevia + iBatch * nSize;
    for(int i=0; i<nSize; i++) {
        //
        //  参考地址：https://blog.csdn.net/jiongjiongai/article/details/88324000
        //              https://blog.csdn.net/qq_42734797/article/details/110748836
        //
        pInDevia[i] += pOutDevia[i];
    }
}
