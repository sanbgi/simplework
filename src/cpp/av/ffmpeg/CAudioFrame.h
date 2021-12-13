#ifndef __SimpleWork_Av_CAudioFrame_h__
#define __SimpleWork_Av_CAudioFrame_h__

#include "av_ffmpeg.h"
#include "CAvFrame.h"

FFMPEG_NAMESPACE_ENTER

class CAudioFrame : public CAvFrame, public IAudioFrame {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CAvFrame)
        SIMPLEWORK_INTERFACE_ENTRY(IAudioFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CAvFrame)

public://IAvFrame
    SAvFrame::AvFrameType getFrameType();

public://IAudioFrame
    STensor getFrameAudioSamples(SAudioFrame::AvFrameSampleType eType, int nSampleRate, int nChannels);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAudioFrame_h__