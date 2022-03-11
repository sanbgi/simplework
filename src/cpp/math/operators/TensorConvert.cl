

kernel void uc2floatEval(
    int nIn1, global unsigned char * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void uc2doubleEval(
    int nIn1, global unsigned char * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void int2floatEval(
    int nIn1, global int * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}

kernel void int2doubleEval(
    int nIn1, global int * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn[gid];
}
