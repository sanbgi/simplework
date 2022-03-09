#include "../inc/math/math.h"
#include <map>
#include "CTensor.h"
#include "CDimension.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 张量基类，主要用于申明不带模板参数的初始化函数
//
class CMathFactory : public CObject, IMathFactory {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IMathFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createVector(STensor& spTensor, PDATATYPE eElementType, int nElementSize, const void* pElementData = nullptr) {
        return CTensor::createTensor(spTensor, nullptr, eElementType, nElementSize, pElementData);
    }
    
    int createTensor(STensor& spTensor, const SDimension& spDimVector, PDATATYPE eElementType, int nElementSize, const void* pElementData = nullptr ){
        return CTensor::createTensor(spTensor, &spDimVector, eElementType, nElementSize, pElementData);
    }

    int createDimension(SDimension& spDimension, int nElementSize, const int* pElementData) {
        return CDimension::createDimension(spDimension, nElementSize, pElementData);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMathFactory, SMathFactory::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE