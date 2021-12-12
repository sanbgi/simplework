#include "CVideoFrame.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

AvFrame::AvFrameType CVideoFrame::getFrameType() {
    return AvFrame::AVSTREAMTYPE_VIDEO;
}

Tensor CVideoFrame::getFrameVideoImage(VideoFrame::AvFrameImageType eType) {
    AVPixelFormat ePixFormat;
    switch(eType) {
    case VideoFrame::AVFRAMEIMAGETYPE_RGB:
        ePixFormat = AV_PIX_FMT_RGB24;
        break;

    case VideoFrame::AVFRAMEIMAGETYPE_RGBA:
        ePixFormat = AV_PIX_FMT_RGBA;
        break;

    default:
        return Tensor();
    }
    return m_pStreaming->convertImage(m_pAvFrame, ePixFormat);
}

FFMPEG_NAMESPACE_LEAVE