#include "Av.h"
#include "CAvFrame.h"

using namespace sw::math;

AvStreaming::AvStreamingType CAvFrame::getStreamingType() {
    return m_spAvStream->getStreamingType();
}

AvStreaming& CAvFrame::getStreaming() {
    return m_spAvStream;
}

Tensor CAvFrame::getVideoImage(AvFrame::AvFrameImageType eImageType) { 
    return Tensor();
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

