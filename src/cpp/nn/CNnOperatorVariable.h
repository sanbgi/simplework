#ifndef __SimpleWork_NN_COperatorVariable_H__
#define __SimpleWork_NN_COperatorVariable_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;

class CNnOperatorVariable : public CNnVariable {
public:
    //
    // 创建求解计算器 
    //
    static int createOperatorVariable(const SDimension& spDimension, SNnVariable& spOutVar);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVOperator; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType) {
        return nullptr;
    }
};

#endif//__SimpleWork_NN_COperatorVariable_H__
