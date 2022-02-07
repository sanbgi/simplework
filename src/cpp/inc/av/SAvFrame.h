#ifndef __SimpleWork_AV_AvFrame_h__
#define __SimpleWork_AV_AvFrame_h__

#include "av.h"
#include "PAvFrame.h"
#include "SAvFactory.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(AvFrame)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IAvFrame", 220109)

        //
        // 获取帧数据指针
        //
        virtual const PAvFrame* getFramePtr() = 0; 

    SIMPLEWORK_INTERFACE_LEAVE


    static SAvFrame loadImageFile(const char* szFileName) {
        SAvFrame spFrame;
        SAvFactory::getAvFactory()->loadAvImageFile(szFileName, spFrame);
        return spFrame;
    }

    static int saveImageFile(const char* szFileName, const SAvFrame& spFrame) {
        return SAvFactory::getAvFactory()->saveAvImageFile(szFileName, spFrame);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvFrame_h__