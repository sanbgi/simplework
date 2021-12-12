#include "CAvFrame.h"
#include "CAvStreaming.h"
#include "CAudioFrame.h"
#include "CVideoFrame.h"

FFMPEG_NAMESPACE_ENTER

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

int CAvFrame::createAvFrame(CFFMpegPointer<AVFrame>& rAvFrame, CAvStreaming* pStreaming, AvFrame& rFrame) {
    Object spObject;
    CAvFrame* pAvFrame;
    switch(pStreaming->m_pCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        pAvFrame = CObject::createObject<CVideoFrame>(spObject);
        break;

    case AVMEDIA_TYPE_AUDIO:
        pAvFrame = CObject::createObject<CAudioFrame>(spObject);
        break;

    default:
        return Error::ERRORTYPE_FAILURE;
    }

    pAvFrame->m_pAvFrame = rAvFrame.detach();
    pAvFrame->m_spAvStream.setPtr((IAvStreaming*)pStreaming);
    pAvFrame->m_pStreaming = pStreaming;
    rFrame.setPtr(pAvFrame);
    return Error::ERRORTYPE_SUCCESS;
}

CAvFrame::CAvFrame() {
    m_pAvFrame = nullptr;
}

CAvFrame::~CAvFrame() {
    if(m_pAvFrame) {
        av_frame_free(&m_pAvFrame);
        m_pAvFrame = nullptr;
    }

}

FFMPEG_NAMESPACE_LEAVE