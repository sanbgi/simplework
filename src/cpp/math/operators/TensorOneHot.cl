
kernel void itofEval(
    int nClassify,
    int nIn, global int * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0f : 0.0f;
}

kernel void itodEval(
    int nClassify,
    int nIn, global int * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0 : 0.0;
}

kernel void uctofEval(
    int nClassify,
    int nIn, global unsigned char * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0f : 0.0f;
}

kernel void uctodEval(
    int nClassify,
    int nIn, global unsigned char * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0 : 0.0;
}

