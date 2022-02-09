#include "CType.h"

using namespace sw;

int CType::getTypeBytes(unsigned int idType) {
    if(idType == CBasicData<double>::getStaticType()) {
        return sizeof(double);
    }else
    if(idType == CBasicData<float>::getStaticType()) {
        return sizeof(float);
    }
    return 0;
}