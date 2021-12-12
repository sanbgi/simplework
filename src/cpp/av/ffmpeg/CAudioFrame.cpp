#include "CAudioFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

AvFrame::AvFrameType CAudioFrame::getFrameType() {
    return AvFrame::AVSTREAMTYPE_AUDIO;
}

Tensor CAudioFrame::getFrameAudioSamples(AudioFrame::AvFrameSampleType eType, int sampleRate, int nChannels) {
    AVSampleFormat eSampleFormat;
    switch(eType) {
    case AudioFrame::AVFRAMESAMPLETYPE_U8:
        eSampleFormat = AV_SAMPLE_FMT_U8;
        break;

    case AudioFrame::AVFRAMESAMPLETYPE_S16:
        eSampleFormat = AV_SAMPLE_FMT_S16;
        break;

    default:
        return Tensor(); 
    }
    return m_pStreaming->convertAudio(m_pAvFrame, eSampleFormat, sampleRate, nChannels);
}

FFMPEG_NAMESPACE_LEAVE