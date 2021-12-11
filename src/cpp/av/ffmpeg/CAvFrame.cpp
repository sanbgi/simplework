#include "CAvFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

AvFrame::AvFrameType CAvFrame::getFrameType() {
    return m_spAvStream->getFrameType();
}

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

Tensor CAvFrame::getFrameVideoImage(AvFrame::AvFrameImageType eType) {
    AVPixelFormat ePixFormat;
    switch(eType) {
    case AvFrame::AVFRAMEIMAGETYPE_RGB:
        ePixFormat = AV_PIX_FMT_RGB24;
        break;

    case AvFrame::AVFRAMEIMAGETYPE_RGBA:
        ePixFormat = AV_PIX_FMT_RGBA;
        break;

    default:
        return Tensor();
    }
    return m_pStreaming->convertImage(m_pAvFrame, ePixFormat);
}

Tensor CAvFrame::getFrameAudioSamples(AvFrame::AvFrameSampleType eType, int sampleRate, int nChannels) {
    AVSampleFormat eSampleFormat;
    switch(eType) {
    case AvFrame::AVFRAMESAMPLETYPE_U8:
        eSampleFormat = AV_SAMPLE_FMT_U8;
        break;

    case AvFrame::AVFRAMESAMPLETYPE_S16:
        eSampleFormat = AV_SAMPLE_FMT_S16;
        break;

    default:
        return Tensor(); 
    }
    return m_pStreaming->convertAudio(m_pAvFrame, eSampleFormat, sampleRate, nChannels);
}

void CAvFrame::attachAvFrame(AVFrame* pAvFrame) {
    if(m_pAvFrame) {
        av_frame_free(&m_pAvFrame);
        m_pAvFrame = nullptr;
    }
    m_pAvFrame = pAvFrame;
}

CAvFrame::CAvFrame() {
    m_pAvFrame = nullptr;
    m_eAvFrameType = AvFrame::AVSTREAMTYPE_UNKNOWN;
}

CAvFrame::~CAvFrame() {
    attachAvFrame(nullptr);
}

FFMPEG_NAMESPACE_LEAVE