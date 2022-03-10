

kernel void uctofEval(
    int nIn1, global unsigned char * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void uctodEval(
    int nIn1, global unsigned char * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void itofEval(
    int nIn1, global int * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void itodEval(
    int nIn1, global int * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}
