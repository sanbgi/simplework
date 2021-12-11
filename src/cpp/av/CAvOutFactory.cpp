
#include "av.h"
#include "CAvOut_SDLWindow.hpp"

class CAvOutFactory : public CObject, public AvOut::IAvOutFactory{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(AvOut::IAvOutFactory)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    AvOut openWindow(const char* szWindowName, int width, int height) {
        ObjectWithPtr<CAvOut_SDLWindow> wrapAvOut = CObject::createObjectWithPtr<CAvOut_SDLWindow>();
        if( wrapAvOut.pObject->initWindow(szWindowName, width, height) != Error::ERRORTYPE_SUCCESS ) {
            return AvOut();
        }
        return AvOut::wrapPtr((IAvOut*)wrapAvOut.pObject);
    }
};

SIMPLEWORK_SINGLETON_FACTORY_REGISTER(CAvOutFactory, AvOut::AvOutFactory::getClassKey())