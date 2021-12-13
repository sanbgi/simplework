#include "CAudioFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

SAvFrame::AvFrameType CAudioFrame::getFrameType() {
    return SAvFrame::AVSTREAMTYPE_AUDIO;
}

STensor CAudioFrame::getFrameAudioSamples(SAudioFrame::AvFrameSampleType eType, int sampleRate, int nChannels) {
    AVSampleFormat eSampleFormat;
    switch(eType) {
    case SAudioFrame::AVFRAMESAMPLETYPE_U8:
        eSampleFormat = AV_SAMPLE_FMT_U8;
        break;

    case SAudioFrame::AVFRAMESAMPLETYPE_S16:
        eSampleFormat = AV_SAMPLE_FMT_S16;
        break;

    default:
        return STensor(); 
    }
    return m_pStreaming->convertAudio(m_spAvFrame, eSampleFormat, sampleRate, nChannels);
}

FFMPEG_NAMESPACE_LEAVE