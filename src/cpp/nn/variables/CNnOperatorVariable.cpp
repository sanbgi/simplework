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

int CNnOperatorVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnOperatorVariable, CNnOperatorVariable::__getClassKey())