#include "Core.h"
#include <string>

__SimpleWork_Core_Namespace_Enter__

class CDataFactory : public CObject, public SData::IDataFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SData::IDataFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    unsigned int getTypeIdentifier(const char* szDataTypeKey) {
        //
        // 是不是hash就足够了？还是说，我要把每一个Key记录下来，然后分配一个值？这样太浪费内存了
        //
        return std::hash<std::string>()(szDataTypeKey);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CDataFactory, SData::SDataFactory::getClassKey())

__SimpleWork_Core_Namespace_Leave__