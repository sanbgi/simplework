#ifndef __SimpleWork_NN_COperator_H__
#define __SimpleWork_NN_COperator_H__

#include "nn.h"
#include "SNnOperator.h"
#include <map>

using namespace sw;
using namespace std;

typedef int (*FCreateOperator)(SNnOperator& spOutVar);

class CNnOperator : public CObject, public INnOperator {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnOperator)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    //
    // 创建求解计算器 
    //
    static int solveOp(const char* szOp, const PData* pData, int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);

public:
    int solveOneEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int solveTwoEleWise(int nInVars, const SNnVariable pInVars[], SNnVariable& spOutVar);
    int createVariable(const SDimension& spDimension, SNnVariable& spOutVar);

public:
    static map<string, FCreateOperator>& getFactories();
    static int regisetOperator(const char* szOperator, FCreateOperator funCreator);

public:
    template<typename T> static int createStaticOperator(SNnOperator& spOperator) {
        static SNnOperator s_operator = CObject::createObject<T>();
        spOperator = s_operator;
        return 0;
    }
    template<typename T> static int createOperator(SNnOperator& spOperator) {
        CPointer<T> spObj;
        CObject::createObject(spObj);
        spOperator.setPtr(spObj.getPtr());
        return 0;
    }
};

class SNnOperatorRegister {
public:
    SNnOperatorRegister(const char* szOperator, FCreateOperator funCreator) {
        CNnOperator::regisetOperator(szOperator, funCreator);
    }
};

#endif//__SimpleWork_NN_COperator_H__
