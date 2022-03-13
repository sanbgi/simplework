kernel void floatEval(
    int nBatchs,
    float beta1Bais,
    float beta2Bais,
    global float * pDeviation,
    global float * pMomentum,
    global float * pVelocity)
{
    const float esp = 1e-8f;
    const float  learnRate = 0.001f;
    const float  beta1 = 0.9f;
    const float  beta2 = 0.999f;
    int gid = get_global_id(0);
    pDeviation += gid, pVelocity += gid, pMomentum += gid;

    (*pDeviation) = (*pDeviation) / nBatchs;
    (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
    (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
    float momentum = (*pMomentum) / ( 1 - beta1Bais);
    float velocity = (*pVelocity) / ( 1 - beta2Bais);
    (*pDeviation) = learnRate * momentum / (sqrt(velocity) + esp);
}

kernel void doubleEval(
    int nBatchs,
    double beta1Bais,
    double beta2Bais,
    global double * pDeviation,
    global double * pMomentum,
    global double * pVelocity)
{
    const double  esp = 1e-8f;
    const double  learnRate = 0.001f;
    const double  beta1 = 0.9f;
    const double  beta2 = 0.999f;
    int gid = get_global_id(0);
    pDeviation += gid, pVelocity += gid, pMomentum += gid;

    (*pDeviation) = (*pDeviation) / nBatchs;
    (*pMomentum) = beta1 * (*pMomentum) + (1-beta1) * (*pDeviation);
    (*pVelocity) = beta2 * (*pVelocity) + (1-beta2) * (*pDeviation) * (*pDeviation);
    double momentum = (*pMomentum) / ( 1 - beta1Bais);
    double velocity = (*pVelocity) / ( 1 - beta2Bais);
    (*pDeviation) = learnRate * momentum / (sqrt(velocity) + esp);
}
