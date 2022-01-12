#ifndef __SimpleWork_AV_VideoDevice_h__
#define __SimpleWork_AV_VideoDevice_h__

#include "av_ffmpeg.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(VideoDevice)

    SIMPLEWORK_INTERFACE_ENTER(sw::IObject, "sw.av.IAvIn", 211206)

        //
        // 获取设备名
        //
        virtual const char* getDeviceName() = 0;

        //
        // 获取名字(InputFormat)
        //
        virtual const char* getName() = 0;

        //
        // 获取下一个
        //
        virtual int getNextDevice(SVideoDevice& rDevice) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(VideoDevice)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_VideoDevice_h__