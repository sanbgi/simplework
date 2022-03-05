typedef struct {
}PReluParameter;

kernel void relu_eval(
    global PReluParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid] >= 0 ? pIn[gid] : 0.0f;
}

kernel void relu_devia(
    global PReluParameter* pParameter, 
    int nBatchs,
    int nIn1, global float * pIn, global float * pInDevia, 
    int nOut, global float * pOutDevia)
{
    int gid = get_global_id(0);
    pInDevia[gid] += pIn[gid] >= 0 ? pOutDevia[gid] : 0.0f;
}
