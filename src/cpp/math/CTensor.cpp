#include "../inc/math/math.h"


class CTensor : public CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
};

SIMPLEWORK_FACTORY_REGISTER(CTensor, "sw.math.Tensor")
/*
class __CRegister { 
public: 
    __CRegister() { 
        IFactoryPtr spFactory = CObject::createFactory<CTensor>(); 
        getSimpleWork()->registerFactory("sw.math.Tensor", spFactory); 
    } 
} __gRegister;
*/