
#include "av.h"
#include "ffmpeg/CAvIn.h"

class CAvInFactory : public CObject, public AvIn::IAvInFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(AvIn::IAvInFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvIn openVideoFile(const char* szFileName) {
        Object spAvIn;
        ffmpeg::CAvIn* pAvIn = CObject::createObject<ffmpeg::CAvIn>(spAvIn);
        if( pAvIn->initVideoFile(szFileName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)pAvIn);
    }

    AvIn openVideoCapture(const char* szName) {
        Object spAvIn;
        ffmpeg::CAvIn* pAvIn = CObject::createObject<ffmpeg::CAvIn>(spAvIn);
        if( pAvIn->initVideoCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return AvIn();
        }
        return AvIn::wrapPtr((IAvIn*)pAvIn);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvInFactory, AvIn::AvInFactory::getClassKey())