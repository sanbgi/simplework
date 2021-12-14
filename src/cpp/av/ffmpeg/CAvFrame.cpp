#include "CAvFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

SAvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}
STensor& CAvFrame::getData(){
    m_spData.release();
    m_pStreaming->convertToTensor(m_spData, m_spAvFrame);
    return m_spData;
}

int CAvFrame::createAvFrame(CTaker<AVFrame*>& spAvFrame, CAvStreaming* pStreaming, int& iStreamingId, SAvFrame& rFrame) {
    SObject spObject;
    CAvFrame* pAvFrame;
    switch(pStreaming->m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        pAvFrame = CObject::createObject<CAvFrame>(spObject);
        break;

    case AVMEDIA_TYPE_AUDIO:
        pAvFrame = CObject::createObject<CAvFrame>(spObject);
        break;

    default:
        return Error::ERRORTYPE_FAILURE;
    }

    pAvFrame->m_spAvFrame.take(spAvFrame);
    pAvFrame->m_spAvStream.setPtr((IAvStreaming*)pStreaming);
    pAvFrame->m_pStreaming = pStreaming;
    rFrame.setPtr(pAvFrame);
    iStreamingId = pStreaming->m_iStreamingIndex;
    return Error::ERRORTYPE_SUCCESS;
}

FFMPEG_NAMESPACE_LEAVE