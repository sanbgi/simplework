#ifndef __SimpleWork_Core_CCtx_h__
#define __SimpleWork_Core_CCtx_h__

#include "Core.h"
#include <string>

__SimpleWork_Core_Namespace_Enter__

class CCtx : public CObject, ICtx {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICtx)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    void logError(const char* szError);
    void logWarning(const char* szWarning);
    void logMessage(const char* szMessage);

public:
    static int createCtx(const char* szModule, SCtx& spCtx);
    static SCtx createCtx(const char* szModule);

private:
    std::string m_strModule;
};

__SimpleWork_Core_Namespace_Leave__

#endif//__SimpleWork_Core_CCtx_h__