
#include "av.h"
#include "ffmpeg/CAvIn.h"
#include "ffmpeg/CAvDevice.h"

class CAvInFactory : public CObject, public SAvIn::IAvInFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvIn::IAvInFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    SAvIn openVideoFile(const char* szFileName) {
        SObject spAvIn;
        ffmpeg::CAvIn* pAvIn = CObject::createObject<ffmpeg::CAvIn>(spAvIn);
        if( pAvIn->initVideoFile(szFileName) != Error::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr((IAvIn*)pAvIn);
    }

    SAvIn openVideoDevice(const char* szName) {
        SObject spAvIn;
        ffmpeg::CAvIn* pAvIn = CObject::createObject<ffmpeg::CAvIn>(spAvIn);
        if( pAvIn->initVideoCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr((IAvIn*)pAvIn);
    }

    SAvIn openAudioDevice(const char* szName) {
        SObject spAvIn;
        ffmpeg::CAvIn* pAvIn = CObject::createObject<ffmpeg::CAvIn>(spAvIn);
        if( pAvIn->initAudioCapture(szName) != Error::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr((IAvIn*)pAvIn);
    }

    int getAudioDevice(SAudioDevice& rDevice) {
        return ffmpeg::CAvDevice::s_getNextDevice(rDevice);
    }

    int getVideoDevice(SVideoDevice& rDevice) {
        return ffmpeg::CAvDevice::s_getNextDevice(rDevice);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvInFactory, SAvIn::SAvInFactory::getClassKey())