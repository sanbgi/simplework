#include "Core.h"
#include "CCtx.h"
#include "CString.h"

__SimpleWork_Core_Namespace_Enter__

static SCtx sCtx = CCtx::createCtx("CString"); 

const char* CString::getCharPtr() {
    return m_str.c_str();
}

int CString::createString(const char* sz, SString& spString) {
    CPointer<CString> spPointer;
    CObject::createObject<CString>(spPointer);
    spString.setPtr(spPointer.getPtr());
    spPointer->m_str = sz;
    return spString ? SError::ERRORTYPE_SUCCESS : SError::ERRORTYPE_FAILURE;
}

__SimpleWork_Core_Namespace_Leave__