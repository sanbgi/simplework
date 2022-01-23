#ifndef __SimpleWork_NN_CNnWeight_H__
#define __SimpleWork_NN_CNnWeight_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;
using namespace std;

class CNnWeight : public CNnVariable {

public:
    static int createWeightVariable(const STensor& spDimVector, SNnVariable& spOutVar);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVWeight; }
    int getSubVariables(SNnInternalVariable pSubVariables[4]) { return 0; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
    STensor m_spData;
};
#endif//__SimpleWork_NN_CNnWeight_H__
