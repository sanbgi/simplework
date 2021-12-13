#ifndef __SimpleWork_AV_AudioDevice_h__
#define __SimpleWork_AV_AudioDevice_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AudioDevice)

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAvIn", 211206)

        //
        // 获取流
        //
        virtual const char* getDeviceName() = 0;

        //
        // 获取下一个
        //
        virtual int getNextDevice(SAudioDevice& rDevice) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AudioDevice)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AudioDevice_h__