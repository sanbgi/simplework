#ifndef __SimpleWork_NN_COperatorVariable_H__
#define __SimpleWork_NN_COperatorVariable_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;

class CNnOperatorVariable : public CNnVariable, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnVariable)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Variable"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Variable"; }

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
 