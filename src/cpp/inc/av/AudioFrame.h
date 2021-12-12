#ifndef __SimpleWork_AV_AudioFrame_h__
#define __SimpleWork_AV_AudioFrame_h__

#include "av.h"

SIMPLEWORK_AV_NAMESPACE_ENTER 

//
// 对象工场对象的接口
//
SIMPLEWORK_INTERFACECLASS_ENTER0(AudioFrame)
public:
    enum AvFrameSampleType {
        AVFRAMESAMPLETYPE_U8 = 1,
        AVFRAMESAMPLETYPE_S16 = 2,
    };

    SIMPLEWORK_INTERFACE_ENTER(sw::core::IObject, "sw.av.IAudioFrame", 211206)

        //
        // 获取帧音频的数据张量
        //
        virtual sw::math::Tensor getFrameAudioSamples(AvFrameSampleType eType, int sampleRate, int nChannels) = 0;

    SIMPLEWORK_INTERFACE_LEAVE

SIMPLEWORK_INTERFACECLASS_LEAVE(AudioFrame)

SIMPLEWORK_AV_NAMESPACE_LEAVE

#endif//__SimpleWork_AV_AudioFrame_h__