#ifndef __SimpleWork_NN_CNnInput_H__
#define __SimpleWork_NN_CNnInput_H__

#include "variable.h"

class CNnInputVariable : public CNnVariable, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnVariable)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Input"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Input"; }

public:
    static int createVariable(const STensor& spDimension, SNnVariable& spOutVar);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVInput; }
    int getSubVariables(SNnInternalVariable pSubVariables[4]) { return 0; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int) {
        return nullptr;
    }
};
#endif//__SimpleWork_NN_CNnInput_H__
