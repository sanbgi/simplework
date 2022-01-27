#ifndef __SimpleWork_NN_CNnWeight_H__
#define __SimpleWork_NN_CNnWeight_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;
using namespace std;

class CNnWeightVariable : public CNnVariable, public IIoArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CNnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(IIoArchivable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CNnVariable)

private://IIoArchivable
    int getClassVer() { return 220112; }
    const char* getClassName() { return "Weight"; } 
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SIoArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Weight"; }
    static int createWeightVariable(const SDimension& spDimension, SNnVariable& spOutVar);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVWeight; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
    template<typename Q> void initWeightT(int nWeights, void* pWeights);

private:
    STensor m_spData;
};
#endif//__SimpleWork_NN_CNnWeight_H__
