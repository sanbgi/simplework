#ifndef __SimpleWork_SMathFactory_h__
#define __SimpleWork_SMathFactory_h__

SIMPLEWORK_MATH_NAMESPACE_ENTER

class STensor;
class SDimension;
class SMathSolver;

//
// 向量求解器，其输入输出均为相同数据类型
//
SIMPLEWORK_INTERFACECLASS_ENTER(MathFactory, "sw.math.MathFactory")

    SIMPLEWORK_INTERFACE_ENTER(IObject, "sw.math.ISMathFactory", 220118)
        //
        // 构造全新的张量
        //
        virtual int createVector(STensor& spTensor, unsigned int eElementType, int nElementSize, void* pElementData = nullptr) = 0;
        virtual int createTensor(STensor& spTensor, const SDimension& spDimVector, unsigned int eElementType, int nElementSize, void* pElementData = nullptr ) = 0;
        virtual int createDimension(SDimension& spDimension, int nElementSize, const int* pElementData) = 0;

        virtual int createSolver(unsigned int idType, SMathSolver& spSolver) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

public:
    static SMathFactory& getFactory() {
        static SMathFactory g_solver = SObject::createObject<SMathFactory>();
        return g_solver;
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(MathFactory)

SIMPLEWORK_MATH_NAMESPACE_LEAVE

#endif//__SimpleWork_SMathFactory_h__