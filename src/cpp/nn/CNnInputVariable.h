#ifndef __SimpleWork_NN_CNnInput_H__
#define __SimpleWork_NN_CNnInput_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;
using namespace std;

class CNnInputVariable : public CNnVariable {

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
