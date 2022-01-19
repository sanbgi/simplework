
#ifndef __SimpleWork_Math_CTensorFactory_H__
#define __SimpleWork_Math_CTensorFactory_H__

#include "math.h"
#include <map>

using namespace sw;
using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

//
// 定义一个张量工厂类
//
class CTensorFactory{
public:

    //
    // 创建向量
    //
    static int createVector( STensor& spTensor, unsigned int idElementType, int nElementSize, void* pElementData);

    //
    // 创建张量
    //
    static int createTensor( STensor& spTensor, const STensor& spDimVector, unsigned int idElementType, int nElementSize, void* pElementData);
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_Math_CTensorFactory_H__