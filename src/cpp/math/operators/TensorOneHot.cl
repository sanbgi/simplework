
kernel void int2floatEval(
    int nClassify,
    int nIn, global int * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0f : 0.0f;
}

kernel void int2doubleEval(
    int nClassify,
    int nIn, global int * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0 : 0.0;
}

kernel void uchar2floatEval(
    int nClassify,
    int nIn, global unsigned char * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0f : 0.0f;
}

kernel void uchar2doubleEval(
    int nClassify,
    int nIn, global unsigned char * pIn,
    int nOut, global double * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = (pIn[gid/nClassify] == gid%nIn) ? 1.0 : 0.0;
}

