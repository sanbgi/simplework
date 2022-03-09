
kernel void eval(
    int nIn1, global float * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid] * pIn[gid];
}

kernel void devia(
    int nIn1, global float * pIn, global float * pInDevia,
    int nOut, global float * pOutDevia)
{
}
