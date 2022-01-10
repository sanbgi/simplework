#ifndef __SimpleWork_AV_AvFrame_h__
#define __SimpleWork_AV_AvFrame_h__

#include "av.h"
#include "PAvFrame.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvFrame", 220109)

        //
        // 获取帧数据指针
        //
        virtual const PAvFrame* getFramePtr() = 0; 

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvFrame_h__