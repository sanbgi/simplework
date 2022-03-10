
kernel void floatEval(
    int nIn1, global float * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid] * pIn[gid];
}

kernel void floatDevia(
    int nIn1, global float * pIn, global float * pInDevia,
    int nOut, global float * pOut, global float * pOutDevia)
{
}
