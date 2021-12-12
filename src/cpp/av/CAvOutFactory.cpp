
#include "av.h"
#include "CAvOut_SDLWindow.h"
#include "CAvOut_SDLSpeaker.h"

class CAvOutFactory : public CObject, public AvOut::IAvOutFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(AvOut::IAvOutFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvOut openWindow(const char* szWindowName, int width, int height) {
        Object spAvOut;
        CAvOut_SDLWindow* pAvOut = CObject::createObject<CAvOut_SDLWindow>(spAvOut);
        if( pAvOut->initWindow(szWindowName, width, height) != Error::ERRORTYPE_SUCCESS ) {
            return AvOut();
        }
        return AvOut::wrapPtr((IAvOut*)pAvOut);
    }

    AvOut openSpeaker(const char* szName, int sampleRate, int nChannels) {
        Object spAvOut;
        CAvOut_SDLSpeaker* pAvOut = CObject::createObject<CAvOut_SDLSpeaker>(spAvOut);
        if( pAvOut->initSpeaker(szName, sampleRate, nChannels) != Error::ERRORTYPE_SUCCESS ) {
            return AvOut();
        }
        return AvOut::wrapPtr((IAvOut*)pAvOut);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvOutFactory, AvOut::AvOutFactory::getClassKey())