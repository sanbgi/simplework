
kernel void floatEval(
    int nIn1, global float * pIn1,
    int nIn2, global float * pIn2,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    pOut[gid] = pIn1[gid%nIn1] - pIn2[gid%nIn2];
}

kernel void floatDevia(
    int nIn1, global float * pIn1, global float * pIn1Devia,
    int nIn2, global float * pIn2, global float * pIn2Devia,
    int nOut, global float * pOut, global float * pOutDevia)
{

}
