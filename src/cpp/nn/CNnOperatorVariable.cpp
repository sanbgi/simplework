#include "CNnOperatorVariable.h"
#include <map>

static SCtx sCtx("CNnOperatorVariable");
int CNnOperatorVariable::createOperatorVariable(const SDimension& spDimension, SNnVariable& spOutVar) {
    CPointer<CNnOperatorVariable> spVar;
    CObject::createObject(spVar);
    spVar->m_spDimension = spDimension;
    spOutVar.setPtr(spVar.getPtr());
    return sCtx.success();
}
