#include "nn.h"
#include "CUtils.h"

#include <time.h>
#include <math.h>

double CUtils::rand() {
    return ::rand() % 10000 / 10000.0;
}


bool CUtils::isSameDimVector(const STensor& spDimVector1, const STensor& spDimVector2) {
    int size = spDimVector1.size();
    if(spDimVector2.size() != size) {
        return false;
    }

    int* pDimSize1 = spDimVector1.data<int>();
    int* pDimSize2 = spDimVector2.data<int>();
    while(size>0) {
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