
#include "Core.h"

#include <map>
#include <string>
using namespace std;

__SimpleWork_Core_Namespace_Enter__

class CNamedMap : public CObject, public INamedMap{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INamedMap)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

private://INamedMap

    NamedMap createMap() {
        return CObject::createObject<CNamedMap>();
    }

    Object getAt(const char* szName) {
        map<string, Object>::iterator it = m_map.find(szName);
        if(it != m_map.end()) {
            return it->second;
        }else{
            return Object();
        }
    }

    void putAt(const char* szName, Object& obj) {
        if(obj) {
            m_map[szName] = obj;
        }else{
            m_map.erase(szName);
        }
    }

private:
    map<string, Object> m_map;
};

SIMPLEWORK_FACTORY_REGISTER(CNamedMap, NamedMap::getClassKey())

__SimpleWork_Core_Namespace_Leave__
