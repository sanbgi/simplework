#include "io.h"
#include "CIoBinaryArReader.h"
#include "CIoBinaryArWriter.h"

using namespace sw;
using namespace sw;

class CIoFactory : public CObject, public IIoFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IIoFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int saveArchive(const char* szFileName, const SIoArchivable& spAr) {
        return CIoBinaryArWriter::saveArchive(szFileName, spAr);
    }

    int loadArchive(const char* szFileName, SIoArchivable& spAr) {
        return CIoBinaryArReader::loadArchive(szFileName, spAr);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_AUTO_REGISTER(CIoFactory, SIoFactory::__getClassKey())