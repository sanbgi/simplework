#include "../inc/Tensor.h"


class CTensor : CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)
};

SIMPLEWORK_FACTORY_REGISTER(CTensor, "sw.math.Tensor")
