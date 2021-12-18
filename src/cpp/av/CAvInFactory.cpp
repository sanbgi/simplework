
#include "av.h"
#include "ffmpeg/CAvIn.h"
#include "ffmpeg/CAvDevice.h"

class CAvInFactory : public CObject, public SAvIn::IAvInFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvIn::IAvInFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    SAvIn openVideoFile(const char* szFileName) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoFile(szFileName) != SError::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr(spAvIn);
    }

    SAvIn openVideoDevice(const char* szName) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initVideoCapture(szName) != SError::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr(spAvIn);
    }

    SAvIn openAudioDevice(const char* szName) {
        CPointer<ffmpeg::CAvIn> spAvIn;
        CObject::createObject(spAvIn);
        if( spAvIn->initAudioCapture(szName) != SError::ERRORTYPE_SUCCESS ) {
            return SAvIn();
        }
        return SAvIn::wrapPtr(spAvIn);
    }

    int getAudioDevice(SAudioDevice& rDevice) {
        return ffmpeg::CAvDevice::s_getNextDevice(rDevice);
    }

    int getVideoDevice(SVideoDevice& rDevice) {
        return ffmpeg::CAvDevice::s_getNextDevice(rDevice);
    }
};
SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvInFactory, SAvIn::SAvInFactory::getClassKey())