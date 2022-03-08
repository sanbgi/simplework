#include "Core.h"
#include "CCtx.h"
#include "CString.h"
#include <string>
#include <map>

__SimpleWork_Core_Namespace_Enter__

class CCore : public CObject, public ICore{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(ICore)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int createCtx(const char* szModule, SCtx& spCtx) {
        return CCtx::createCtx(szModule, spCtx);
    }

    PID getRuntimeId(const char* szKey) {
        static std::map<std::string, PID> s_runtimeKeys;
        static PID g_runtimeId = 1;
        std::string strKey(szKey==nullptr?"":szKey);
        auto it = s_runtimeKeys.find(strKey);
        if( it != s_runtimeKeys.end() ) {
            return it->second;
        }
        PID pid = g_runtimeId++;
        s_runtimeKeys[strKey] = pid;
        return pid;
    }

    PDATATYPE getDataType(const char* szDataTypeKey) {
        if(szDataTypeKey == nullptr) {
            return 0;
        }
        return std::hash<std::string>()(szDataTypeKey);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CCore, SCore::__getClassKey())

__SimpleWork_Core_Namespace_Leave__