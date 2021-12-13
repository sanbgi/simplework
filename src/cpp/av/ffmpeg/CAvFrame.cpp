#include "CAvFrame.h"
#include "CAvStreaming.h"
#include "CAudioFrame.h"
#include "CVideoFrame.h"

FFMPEG_NAMESPACE_ENTER

SAvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

int CAvFrame::getStreamingId() {
    return m_spAvStream->getStreamingId();
}

int CAvFrame::createAvFrame(CTaker<AVFrame*>& spAvFrame, CAvStreaming* pStreaming, SAvFrame& rFrame) {
    SObject spObject;
    CAvFrame* pAvFrame;
    switch(pStreaming->m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        pAvFrame = CObject::createObject<CVideoFrame>(spObject);
        break;

    case AVMEDIA_TYPE_AUDIO:
        pAvFrame = CObject::createObject<CAudioFrame>(spObject);
        break;

    default:
        return Error::ERRORTYPE_FAILURE;
    }

    pAvFrame->m_spAvFrame.take(spAvFrame);
    pAvFrame->m_spAvStream.setPtr((IAvStreaming*)pStreaming);
    pAvFrame->m_pStreaming = pStreaming;
    rFrame.setPtr(pAvFrame);
    return Error::ERRORTYPE_SUCCESS;
}

FFMPEG_NAMESPACE_LEAVE