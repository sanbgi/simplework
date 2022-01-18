#ifndef __SimpleWork_NNUNIT_CNnUnit_H__
#define __SimpleWork_NNUNIT_CNnUnit_H__

#include "../nn.h"
#include "../COptimizer.h"
#include "../CActivator.h"
#include "../CUtils.h"
#include "../CType.h"
#include "SNnUnit.h"
#include <string>
using namespace sw;
using namespace std;

#define TYPE_ERROR -1;
#define CNUUNIT_INNUNIT_IMPLEMENTATION \
\
int eval(unsigned int idType, const void* pIn, void* pOut){\
    if(idType != m_idType) {\
        return TYPE_ERROR;\
    }\
    if(idType == CBasicData<double>::getStaticType()) {\
        return evalT<double>((const double*)pIn, (double*)pOut);\
    }else\
    if(idType == CBasicData<float>::getStaticType()) {\
        return evalT<float>((const float*)pIn, (float*)pOut);\
    }\
    return TYPE_ERROR;\
}\
\
int learn(unsigned int idType, const void* pIn, const void* pOut, const void* pOutDev, void* pInDev){\
    if(idType != m_idType) {\
        return TYPE_ERROR;\
    }\
    if(idType == CBasicData<double>::getStaticType()) {\
        return learnT<double>((const double*)pIn, (const double*)pOut, (const double*)pOutDev, (double*)pInDev);\
    }else\
    if(idType == CBasicData<float>::getStaticType()) {\
        return learnT<float>((const float*)pIn, (const float*) pOut, (const float*)pOutDev, (float*)pInDev);\
    }\
    return TYPE_ERROR;\
}\
\
int updateWeights(){\
    if(m_idType == CBasicData<double>::getStaticType()) {\
        return updateWeightsT<double>();\
    }else\
    if(m_idType == CBasicData<float>::getStaticType()) {\
        return updateWeightsT<float>();\
    }\
    return TYPE_ERROR;\
}

class CNnUnit {
public:
    static int createBais(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer);
    static int createDense(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szOptimizer, const STensor& spInDim);
    static int createActivator(SNnUnit& spUnit, int nCells, unsigned int idType, const char* szActivator);
};

#endif//__SimpleWork_NNUNIT_CNnUnit_H__