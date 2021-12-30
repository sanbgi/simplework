#include "Core.h"
#include "CCtx.h"
#include "CString.h"
#include <string>

__SimpleWork_Core_Namespace_Enter__

class CCoreFactory : public CObject, public ICoreFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICoreFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createString(const char* sz, SString& spString) {
        return CString::createString(sz, spString);
    }

    int createCtx(const char* szModule, SCtx& spCtx) {
        return CCtx::createCtx(szModule, spCtx);
    }

    unsigned int getTypeIdentifier(const char* szDataTypeKey) {
        return std::hash<std::string>()(szDataTypeKey);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CCoreFactory, SCoreFactory::getClassKey())

__SimpleWork_Core_Namespace_Leave__