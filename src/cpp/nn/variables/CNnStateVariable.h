#ifndef __SimpleWork_NN_CNnState_H__
#define __SimpleWork_NN_CNnState_H__

#include "variable.h"

class CNnStateVariable : public CNnVariable, public INnState, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(INnState)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnVariable)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "State"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://INnState
    int getDimension(SDimension& spDimension) {
        return CNnVariable::getDimension(spDimension);
    }

public://Factory
    static const char* __getClassKey() { return "sw.nn.State"; }
    static int createState(const SDimension& spDimension, SNnState& spOutVar);
    
private:
    ENnVariableType getVariableType() { return ENnVariableType::EVState; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
    STensor m_spData;
};
#endif//__SimpleWork_NN_CNnState_H__
