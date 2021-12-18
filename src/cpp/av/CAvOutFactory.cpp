
#include "av.h"
#include "sdl/CAvOut_SDLWindow.h"
#include "sdl/CAvOut_SDLSpeaker.h"
#include "ffmpeg/CAvOut.h"

class CAvOutFactory : public CObject, public SAvOut::IAvOutFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvOut::IAvOutFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    SAvOut openWindow(const char* szWindowName, PAvSample& sampleMeta) {
        CPointer<sdl::CAvOut_SDLWindow> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initWindow(szWindowName, sampleMeta) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr(spAvOut);
    }

    SAvOut openSpeaker(const char* szName, PAvSample& sampleMeta) {
        CPointer<sdl::CAvOut_SDLSpeaker> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initSpeaker(szName, sampleMeta) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr(spAvOut);
    }

    SAvOut openAvFile(const char* szFileName, int nStreamings, PAvStreaming* pStreamings) {
        CPointer<ffmpeg::CAvOut> spAvOut;
        CObject::createObject(spAvOut);
        if( spAvOut->initAvFile(szFileName, nStreamings, pStreamings) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr(spAvOut);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvOutFactory, SAvOut::SAvOutFactory::getClassKey())