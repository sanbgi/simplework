kernel void floatEval(
    int iWeightOffset,
    int iDeviationOffset,
    float fMin, float fMax,
    global float * pWeights,
    global float * pDeviations)
{
    pWeights += iWeightOffset;
    pDeviations += iDeviationOffset;
    int gid = get_global_id(0);
    float dWeights = pWeights[gid] - pDeviations[gid];
    if(dWeights > fMax) {
        dWeights = fMax;
    }else if( dWeights < fMin) {
        dWeights = fMin;
    }
    pWeights[gid] = dWeights;
}

kernel void doubleEval(
    int iWeightOffset,
    int iDeviationOffset,
    double fMin, double fMax,
    global double * pWeights,
    global double * pDeviations)
{
    pWeights += iWeightOffset;
    pDeviations += iDeviationOffset;
    int gid = get_global_id(0);
    double dWeights = pWeights[gid] - pDeviations[gid];
    if(dWeights > fMax) {
        dWeights = fMax;
    }else if( dWeights < fMin) {
        dWeights = fMin;
    }
    pWeights[gid] = dWeights;
}

