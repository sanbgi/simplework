#ifndef __SimpleWork_NN_CUtils_H__
#define __SimpleWork_NN_CUtils_H__

#include "nn.h"

using namespace sw;

class CUtils {
public:
    static double rand(); 
    static bool isSameDimVector(const STensor& spDimVector1, const STensor& spDimVector2);


    //
    // 反向计算偏导数，注意计算模式
    //  1，权重偏导数会累加
    //  2，输入偏导数会赋值
    //
    //template<typename Q> void bpDeviation(Q* pOut, Q* pOutDeviation, Q* pIn, Q* pInDeviation, Q* pWeight, Q* pWeightDeviation, int nIn, int nOut);

};

#endif//__SimpleWork_NN_CUtils_H__
