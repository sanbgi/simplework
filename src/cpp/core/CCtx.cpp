#include <vector>
#include <iostream>
#include "CCtx.h"

__SimpleWork_Core_Namespace_Enter__

void CCtx::logError(const char* szError) {
    if(szError != nullptr)
        std::cout << "Error: " << m_strModule << "," << szError << "\n";
}

void CCtx::logWarning(const char* szWarning) {
    if(szWarning != nullptr)
        std::cout << "Warning: " << m_strModule << "," << szWarning << "\n";
}

void CCtx::logMessage(const char* szMessage){
    if(szMessage != nullptr)
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

__SimpleWork_Core_Namespace_Leave__