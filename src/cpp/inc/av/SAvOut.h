#ifndef __SimpleWork_AV_AvOut_h__
#define __SimpleWork_AV_AvOut_h__

#include "av.h"
#include "SAvFrame.h"
#include "SAvFactory.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

SIMPLEWORK_INTERFACECLASS_ENTER0(AvOut)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvOut", 220109)

        //
        // 写入视频帧
        //
        virtual int writeFrame(const SAvFrame& avFrame) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

    static SAvOut openAvFileWriter(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings) {
        SAvOut spAvOut;
        SAvFactory::getAvFactory()->openAvFileWriter(szFileName, nStreamings, pStreamings, spAvOut);
        return spAvOut;
    }

    static SAvOut openSpeaker(const char* szDeviceName, PAvSample audioSample) {
        SAvOut spAvOut;
        SAvFactory::getAvFactory()->openSpeaker(szDeviceName, audioSample, spAvOut);
        return spAvOut;
    }

    static SAvOut openWindow(const char* szWindowName, int nWidth, int nHeight) {
        SAvOut spAvOut;
        SAvFactory::getAvFactory()->openWindow(szWindowName, nWidth, nHeight, spAvOut);
        return spAvOut;
    }

    static int saveAvImageFile(const char* szFileName, const SAvFrame& spFrame) {
        return SAvFactory::getAvFactory()->saveAvImageFile(szFileName, spFrame);
    }

SIMPLEWORK_INTERFACECLASS_LEAVE(AvOut)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AvFrame_h__