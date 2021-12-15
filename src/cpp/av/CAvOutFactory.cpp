
#include "av.h"
#include "sdl/CAvOut_SDLWindow.h"
#include "sdl/CAvOut_SDLSpeaker.h"
#include "ffmpeg/CAvOut.h"

class CAvOutFactory : public CObject, public SAvOut::IAvOutFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvOut::IAvOutFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    SAvOut openWindow(const char* szWindowName, CAvSampleMeta& sampleMeta) {
        SObject spAvOut;
        sdl::CAvOut_SDLWindow* pAvOut = CObject::createObject<sdl::CAvOut_SDLWindow>(spAvOut);
        if( pAvOut->initWindow(szWindowName, sampleMeta) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr((IAvOut*)pAvOut);
    }

    SAvOut openSpeaker(const char* szName, CAvSampleMeta& sampleMeta) {
        SObject spAvOut;
        sdl::CAvOut_SDLSpeaker* pAvOut = CObject::createObject<sdl::CAvOut_SDLSpeaker>(spAvOut);
        if( pAvOut->initSpeaker(szName, sampleMeta) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr((IAvOut*)pAvOut);
    }

    SAvOut openAvFile(const char* szFileName, int nStreamings, SAvStreaming* pStreamings) {
        SObject spAvOut;
        ffmpeg::CAvOut* pAvOut = CObject::createObject<ffmpeg::CAvOut>(spAvOut);
        if( pAvOut->initAvFile(szFileName, nStreamings, pStreamings) != SError::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr((IAvOut*)pAvOut);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvOutFactory, SAvOut::SAvOutFactory::getClassKey())