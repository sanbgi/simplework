#ifndef __SimpleWork_NN_CNormalizePipe_H__
#define __SimpleWork_NN_CNormalizePipe_H__

#include "nn.h"

using namespace SIMPLEWORK_CORE_NAMESPACE;
using namespace SIMPLEWORK_MATH_NAMESPACE;
using namespace SIMPLEWORK_NN_NAMESPACE;

class CNormalizePipe : public CObject, INnPipe{

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(INnPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int push(const STensor& spIn, STensor& spOut);

public://Factory
    static int createPipe(SNnPipe& spPipe);
};

#endif//__SimpleWork_NN_CNormalizePipe_H__
