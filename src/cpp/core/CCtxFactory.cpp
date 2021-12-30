#include <vector>
#include <iostream>
#include "CCtx.h"

__SimpleWork_Core_Namespace_Enter__

void CCtx::logError(const char* szError) {
    std::cout << "Error: " << m_strModule << "," << szError << "\n";
}

void CCtx::logWarning(const char* szWarning) {
    std::cout << "Warning: " << m_strModule << "," << szWarning << "\n";
}

void CCtx::logMessage(const char* szMessage){
    std::cout << "Message: " << m_strModule << "," << szMessage << "\n";
}

int CCtx::createCtx(const char* szModule, SCtx& spCtx) {
    CPointer<CCtx> spPointer;
    CObject::createObject<CCtx>(spPointer);
    spCtx = spPointer.getObject();
    spPointer->m_strModule = szModule;
    return spCtx ? SError::ERRORTYPE_SUCCESS : SError::ERRORTYPE_FAILURE;
}

SCtx CCtx::createCtx(const char* szModule) {
    CPointer<CCtx> spPointer;
    CObject::createObject<CCtx>(spPointer);
    spPointer->m_strModule = szModule;
    return SCtx::wrapPtr(spPointer.getPtr());
}

class CCtxFactory : public CObject, public SCtx::ICtxFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SCtx::ICtxFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createCtx(const char* szModule, SCtx& spCtx) {
        return CCtx::createCtx(szModule, spCtx);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CCtxFactory, SCtx::SCtxFactory::getClassKey())

__SimpleWork_Core_Namespace_Leave__