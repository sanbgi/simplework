kernel void intEval(
    global int * pDest, int iDestOffset,
    global int * pSrc, int iSrcOffset)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= pSrc[gid+iSrcOffset];
}

kernel void floatEval(
    global float * pDest, int iDestOffset,
    global float * pSrc, int iSrcOffset)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= pSrc[gid+iSrcOffset];
}

kernel void doubleEval(
    global double * pDest, int iDestOffset,
    global double * pSrc, int iSrcOffset)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= pSrc[gid+iSrcOffset];
}

kernel void intEvalV(
    global int * pDest, int iDestOffset, int v)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= v;
}

kernel void floatEvalV(
    global float * pDest, int iDestOffset, float v)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= v;
}

kernel void doubleEvalV(
    global double * pDest, int iDestOffset, double v)
{
    int gid = get_global_id(0);
    pDest[gid+iDestOffset] -= v;
}

