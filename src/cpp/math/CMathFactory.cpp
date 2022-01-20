#include "../inc/math/math.h"
#include <map>
#include "CTensorFactory.h"
#include "CMathSolver.h"

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
    int createVector(STensor& spTensor, unsigned int eElementType, int nElementSize, void* pElementData = nullptr) {
        return CTensorFactory::createVector(spTensor, eElementType, nElementSize, pElementData);
    }
    
    int createTensor(STensor& spTensor, const STensor& spDimVector, unsigned int eElementType, int nElementSize, void* pElementData = nullptr ){
        return CTensorFactory::createTensor(spTensor, spDimVector, eElementType, nElementSize, pElementData);
    }

    int createSolver(unsigned int idType, SMathSolver& spSolver) {
        return CMathSolver::createSolver(idType, spSolver);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CMathFactory, SMathFactory::__getClassKey())

SIMPLEWORK_MATH_NAMESPACE_LEAVE