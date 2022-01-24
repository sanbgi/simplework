#ifndef __SimpleWork_NN_CNnWeight_H__
#define __SimpleWork_NN_CNnWeight_H__

#include "nn.h"
#include "CNnVariable.h"

using namespace sw;
using namespace std;

class CNnWeightVariable : public CNnVariable {

public:
    static int createWeightVariable(const SDimension& spDimension, SNnVariable& spOutVar);

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVWeight; }
    int getSubVariables(SNnInternalVariable pSubVariables[4]) { return 0; }
    CNnVariable* getVariablePtr() { return this; }
    void* getData(unsigned int idType);

private:
    template<typename Q> void initWeightT(int nWeights, void* pWeights);

private:
    STensor m_spData;
};
#endif//__SimpleWork_NN_CNnWeight_H__
