
kernel void eval(
    int nIn, global float * pIn,
    int nOut, global float * pOut)
{
    int gid = get_global_id(0);
    float sum = 0.0f;
    for(int i=0; i<nIn; i++) {
        sum += pIn[i];
    }
    pOut[0] = sum / nIn;
}

kernel void devia(
    void* pPatameter,
    int nIn1, global float * pIn, global float * pInDevia,
    int nOut, global float * pOut, global float * pOutDevia)
{
}
