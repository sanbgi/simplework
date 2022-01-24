#include "CNnInputVariable.h"

static SCtx sCtx("CNnInputVariable");
int CNnInputVariable::createVariable(const STensor& spDimension, SNnVariable& spOutVar) {
    CPointer<CNnInputVariable> spState;
    CObject::createObject(spState);
    spState->m_spDimension = spDimension;
    spOutVar.setPtr(spState.getPtr());
    return sCtx.success();
}