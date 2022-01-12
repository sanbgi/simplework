#ifndef __SimpleWork_AV_AvNetwork_h__
#define __SimpleWork_AV_AvNetwork_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

class SAvFrame;

SIMPLEWORK_INTERFACECLASS_ENTER0(AvNetwork)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IAvNetwork", 220109)

        //
        // 向管道推入帧，并获取返回帧
        //
        virtual int pipeIn(const SAvFrame& spIn, SAvFrame& spOut) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvNetwork)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvNetwork_h__