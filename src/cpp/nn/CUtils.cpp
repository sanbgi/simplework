#include "nn.h"
#include "CUtils.h"

#include <time.h>
#include <math.h>

double CUtils::rand() {
    return ::rand() % 10000 / 10000.0;
}


bool CUtils::isSameDimension(const SDimension& spDimension1, const SDimension& spDimension2) {
    int size = spDimension1.size();
    if(spDimension2.size() != size) {
        return false;
    }

    const int* pDimSize1 = spDimension1.data();
    const int* pDimSize2 = spDimension2.data();
    while(size-->0) {
        if( *pDimSize1 != *pDimSize2 ) {
            return false;
        }
    }
    return true;
}
/*
void CUtils::bpDeviation(Q* pOut, Q* pOutDeviation, Q* pIn, Q* pInDeviation, Q* pWeight, Q* pWeightDeviation, int nIn, int nOut) {

    int iIn, iOut;
    for(iOut=0; iOut<nOut; iOut++) {
        for(iIn=0; )
    }
}*/