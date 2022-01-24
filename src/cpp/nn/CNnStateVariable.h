#ifndef __SimpleWork_NN_CNnState_H__
#define __SimpleWork_NN_CNnState_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;
using namespace std;

class CNnStateVariable : public CNnVariable {

public:
    static int createStateVariable(const SDimension& spDimension, SNnVariable& spOutVar);
    
private:
    ENnVariableType getVariableType() { return ENnVariableType::EVState; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
    STensor m_spData;
};
#endif//__SimpleWork_NN_CNnState_H__
