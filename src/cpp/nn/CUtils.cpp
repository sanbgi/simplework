#include "CUtils.h"
#include <time.h>
#include <math.h>

double CUtils::rand() {
    return ::rand() % 10000 / 10000.0;
}

/*
void CUtils::bpDeviation(Q* pOut, Q* pOutDeviation, Q* pIn, Q* pInDeviation, Q* pWeight, Q* pWeightDeviation, int nIn, int nOut) {

    int iIn, iOut;
    for(iOut=0; iOut<nOut; iOut++) {
        for(iIn=0; )
    }
}*/