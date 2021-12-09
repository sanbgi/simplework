#include "Av.h"
#include "CAvFrame.h"


AvFrame::AvFrameType CAvFrame::getFrameType() {
    return m_eAvFrameType;
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

