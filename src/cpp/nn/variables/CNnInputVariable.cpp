#include "CNnInputVariable.h"

static SCtx sCtx("CNnInputVariable");
int CNnInputVariable::createVariable(const STensor& spDimension, SNnVariable& spOutVar) {
    CPointer<CNnInputVariable> spState;
    CObject::createObject(spState);
    spState->m_spDimension = spDimension;
    spOutVar.setPtr(spState.getPtr());
    return sCtx.success();
}

int CNnInputVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    return sCtx.success();
}

SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnInputVariable, CNnInputVariable::__getClassKey())