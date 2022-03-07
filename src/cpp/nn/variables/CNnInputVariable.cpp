#include "variable.h"
static SCtx sCtx("CNnInputVariable");
class CNnInputVariable : public CObject, public INnVariable, public INnInternalVariable, public IArchivable {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IArchivable)
        SIMPLEWORK_INTERFACE_ENTRY(INnVariable)
        SIMPLEWORK_INTERFACE_ENTRY(INnInternalVariable)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://IArchivable
    int getClassVer() { return 220112; }
    const char* getClassKey() { return __getClassKey(); }
    int toArchive(const SArchive& ar);

public://Factory
    static const char* __getClassKey() { return "sw.nn.Input"; }

public://CObject
   int __initialize(const PData* pData);

public://INnVariable
    int getDimension(SDimension& spDimension) {
        spDimension = m_spDimension;
        return 0;
    }
    int getSize() {
        return m_spDimension.dataSize();
    }

private:
    ENnVariableType getVariableType() { return ENnVariableType::EVInput; }
    void* getData(PID) {
        return nullptr;
    }

protected:
    SDimension m_spDimension;

};

int CNnInputVariable::toArchive(const SArchive& ar) {
    ar.arObject("dimension", m_spDimension);
    return sCtx.success();
}

int CNnInputVariable::__initialize(const PData* pData) {
    const SDimension* pDimension = CData<SDimension>(pData);
    if(pDimension == nullptr) {
        return sCtx.error("缺少构造参数");
    }
    m_spDimension = *pDimension;
    return sCtx.success();
}


SIMPLEWORK_FACTORY_AUTO_REGISTER(CNnInputVariable, CNnInputVariable::__getClassKey())