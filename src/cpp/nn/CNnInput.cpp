#include "CNnInput.h"

static SCtx sCtx("CNnInput");
int CNnInput::createVariable(const STensor& spDimension, SNnVariable& spOutVar) {
    CPointer<CNnInput> spState;
    CObject::createObject(spState);
    spState->m_spDimension = spDimension;
    spOutVar.setPtr(spState.getPtr());
    return sCtx.success();
}