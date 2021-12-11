
#include "av.h"
#include "ffmpeg/CAvIn.h"

class CAvInFactory : public CObject, public AvIn::IAvInFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(AvIn::IAvInFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvIn openVideoFile(const char* szFileName) {
        ObjectWithPtr<ffmpeg::CAvIn> wrapAvOut = CObject::createObjectWithPtr<ffmpeg::CAvIn>();
        if( wrapAvOut.pObject->initVideoFile(szFileName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }

    AvIn openVideoCapture(const char* szName) {
        ObjectWithPtr<ffmpeg::CAvIn> wrapAvOut = CObject::createObjectWithPtr<ffmpeg::CAvIn>();
        if( wrapAvOut.pObject->initVideoCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)wrapAvOut.pObject);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvInFactory, AvIn::AvInFactory::getClassKey())