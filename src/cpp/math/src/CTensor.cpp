#include "../inc/Tensor.h"


class CTensor : CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
};

class CCRegister {
public:
    CCRegister() {
        IFactoryPtr spFactory = CFactory::createFactory<CTensor>();
        getSimpleWorkModule()->registerFactory("Tensor", spFactory);
    }
} _gRegister;
