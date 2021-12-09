#include "Av.h"
#include "CAvFrame.h"


AvStreaming::AvStreamingType CAvFrame::getStreamingType() {
    return m_spAvStream->getStreamingType();
}

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
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
    m_eAvFrameType = AvFrame::AVFRAMETYPE_UNKNOWN;
}

CAvFrame::~CAvFrame() {
    attachAvFrame(nullptr);
}

