#include "Core.h"
#include "CCtx.h"

#include <string>

__SimpleWork_Core_Namespace_Enter__

static SCtx sCtx = CCtx::createCtx("CString"); 
class CString : public CObject, public IString{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IString)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    const char* getCharPtr() {
        return m_str.c_str();
    }

private:
    std::string m_str;
};

class CStringFactory : public CObject, public SString::IStringFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SString::IStringFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createString(const char* sz, SString& spString) {
        return sCtx.Error();
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CStringFactory, SString::SStringFactory::getClassKey())

__SimpleWork_Core_Namespace_Leave__