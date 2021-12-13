#include "CVideoFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

SAvFrame::AvFrameType CVideoFrame::getFrameType() {
    return SAvFrame::AVSTREAMTYPE_VIDEO;
}

STensor CVideoFrame::getFrameVideoImage(SVideoFrame::AvFrameImageType eType) {
    AVPixelFormat ePixFormat;
    switch(eType) {
    case SVideoFrame::AVFRAMEIMAGETYPE_RGB:
        ePixFormat = AV_PIX_FMT_RGB24;
        break;

    case SVideoFrame::AVFRAMEIMAGETYPE_RGBA:
        ePixFormat = AV_PIX_FMT_RGBA;
        break;

    default:
        return STensor();
    }
    return m_pStreaming->convertImage(m_spAvFrame, ePixFormat);
}

FFMPEG_NAMESPACE_LEAVE