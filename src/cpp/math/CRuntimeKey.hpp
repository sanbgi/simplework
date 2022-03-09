#include "./math.h"

using namespace sw;

SIMPLEWORK_MATH_NAMESPACE_ENTER

class CRuntimeKey {
public:
    CRuntimeKey(const char* szKey){
        m_szKey = szKey;
    }

    bool updateRumtimeKey(PDATATYPE type) {
        if(m_sRuntimeKey.runtimeKey == nullptr) {
            switch(type) {
            case PDATATYPE_FLOAT:
                m_opKey = std::string(m_szKey) + "Float";
                break;

            case PDATATYPE_DOUBLE:
                m_opKey = std::string(m_szKey) + "Double";
                break;

            default:
                return false;
            }

            m_sRuntimeKey.runtimeKey = m_opKey.c_str();
            m_sRuntimeKey.runtimeId = SCore::getFactory()->getRuntimeId(m_sRuntimeKey.runtimeKey);
        }
        return true;
    }

public:
    const char* m_szKey;
    PRuntimeKey m_sRuntimeKey;
    std::string m_opKey;
};

SIMPLEWORK_MATH_NAMESPACE_LEAVE