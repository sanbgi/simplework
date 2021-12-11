
#include "av.h"
#include "CAvIn_ffmpeg.hpp"

class CAvInFactory : public CObject, public AvIn::IAvInFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(AvIn::IAvInFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvIn openVideoFile(const char* szFileName) {
        ObjectWithPtr<CAvIn_ffmpeg> wrapAvOut = CObject::createObjectWithPtr<CAvIn_ffmpeg>();
        if( wrapAvOut.pObject->initVideoFile(szFileName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }

    AvIn openCapture(const char* szName) {
        ObjectWithPtr<CAvIn_ffmpeg> wrapAvOut = CObject::createObjectWithPtr<CAvIn_ffmpeg>();
        if( wrapAvOut.pObject->initCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }
};
SIMPLEWORK_FACTORY_REGISTER(CAvInFactory, AvIn::AvInFactory::getClassKey())