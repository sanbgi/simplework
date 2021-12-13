
#include "av.h"
#include "sdl/CAvOut_SDLWindow.h"
#include "sdl/CAvOut_SDLSpeaker.h"

class CAvOutFactory : public CObject, public SAvOut::IAvOutFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(SAvOut::IAvOutFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    SAvOut openWindow(const char* szWindowName, int width, int height) {
        SObject spAvOut;
        sdl::CAvOut_SDLWindow* pAvOut = CObject::createObject<sdl::CAvOut_SDLWindow>(spAvOut);
        if( pAvOut->initWindow(szWindowName, width, height) != Error::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr((IAvOut*)pAvOut);
    }

    SAvOut openSpeaker(const char* szName, int sampleRate, int nChannels) {
        SObject spAvOut;
        sdl::CAvOut_SDLSpeaker* pAvOut = CObject::createObject<sdl::CAvOut_SDLSpeaker>(spAvOut);
        if( pAvOut->initSpeaker(szName, sampleRate, nChannels) != Error::ERRORTYPE_SUCCESS ) {
            return SAvOut();
        }
        return SAvOut::wrapPtr((IAvOut*)pAvOut);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvOutFactory, SAvOut::SAvOutFactory::getClassKey())