#include "CNnOperatorVariable.h"
#include <map>

static SCtx sCtx("CNnOperatorVariable");
int CNnOperatorVariable::createOperatorVariable(const SDimension& spDimension, const SNnOperator& spOperator, SNnVariable& spOutVar) {
    CPointer<CNnOperatorVariable> spVar;
    CObject::createObject(spVar);
    spVar->m_spDimension = spDimension;
    spVar->m_spOperator = spOperator;
    spOutVar.setPtr(spVar.getPtr());
    return sCtx.success();
}

int CNnOperatorVariable::getSubVariables(SNnInternalVariable pSubVariables[4]) {
    SNnVariable pInVars[4];
    int nInVars = m_spOperator->getInVars(pInVars);
    for(int i=0; i<nInVars; i++) {
        pSubVariables[i] = pInVars[i];
    }
    return nInVars;
}