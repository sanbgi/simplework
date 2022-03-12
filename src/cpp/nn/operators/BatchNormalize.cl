

typedef struct{
    int m_nLayers;
    double m_dEsp;
    int m_nMinBatch;
}PBatchNormalize;

kernel void floatEval(
    global PBatchNormalize* pThis, 
    int nBatchs,
    int nIn, global float* pIn,
    int nOut, global float* pOut)
{
    int iLayer = get_global_id(0);
    int nLayer = pThis->m_nLayers;
    int nItems = nIn / nLayer;
    pIn = pIn + iLayer;
    pOut = pOut + iLayer;

    int i;
    global float *pItIn, *pItOut;
    float avg = 0.0f, x;
    if(nBatchs>=pThis->m_nMinBatch) {
        float sum = 0.0f;
        for(i=0, pItIn=pIn; i<nItems; i++) {
            sum += *pItIn;
            pItIn += nLayer;
        }
        avg = sum/nItems;

        float delta, variance = 0.0f;
        for(i=0, pItIn=pIn; i<nItems; i++) {
            delta = *pItIn - avg;
            variance += delta * delta;
            pItIn += nLayer;
        }
        variance = variance/nItems;
        x = 1.0f/sqrt(variance+pThis->m_dEsp);
    }

    for(i=0, pItIn=pIn, pItOut=pOut; i<nItems; i++) {
        *pItOut = (*pItIn - avg) * x;
        pItIn += nLayer;
        pItOut += nLayer;
    }
}

kernel void floatDevia(
    global PBatchNormalize* pThis, 
    int nBatchs,
    int nIn, global float * pIn, global float * pInDevia,
    int nOut, global float * pOutDevia)
{
    int iLayer = get_global_id(0);
    int nLayer = pThis->m_nLayers;
    int nItems = nIn / nLayer;
    pIn = pIn + iLayer;
    pInDevia = pInDevia + iLayer;
    pOutDevia = pOutDevia + iLayer;
    float sum = 0.0f;

    int i;
    float avg = 0.0f, x = 1.0f;
    if(nBatchs>=pThis->m_nMinBatch) {
        global float *pItIn;
        float sum = 0.0f;
        for(i=0, pItIn=pIn; i<nItems; i++) {
            sum += *pItIn;
            pItIn += nLayer;
        }

        avg = sum/nItems;
        float delta, variance = 0.0f;
        for(i=0, pItIn=pIn; i<nItems; i++) {
            delta = *pItIn - avg;
            variance += delta * delta;
            pItIn += nLayer;
        }
        x = 1.0f/sqrt(variance/nItems+pThis->m_dEsp);
    }

    union {
        unsigned int intVal;
        float floatVal;
    } newVal, prevVal;
    for(i=0; i<nItems; i++) {
        do {
            prevVal.floatVal = (*pInDevia);
            newVal.floatVal = prevVal.floatVal + *pOutDevia * x;
        } while (atomic_cmpxchg((volatile __global unsigned int *)pInDevia, 
                                prevVal.intVal, newVal.intVal) 
                                != prevVal.intVal);
        //(*pInDevia) += *pOutDevia * x;
        pInDevia += nLayer;
        pOutDevia += nLayer;
    }
}
