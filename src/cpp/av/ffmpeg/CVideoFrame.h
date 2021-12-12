#ifndef __SimpleWork_Av_CVideoFrame_h__
#define __SimpleWork_Av_CVideoFrame_h__

#include "av_ffmpeg.h"
#include "CAvFrame.h"

FFMPEG_NAMESPACE_ENTER

class CVideoFrame : public CAvFrame, public IVideoFrame {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CAvFrame)
        SIMPLEWORK_INTERFACE_ENTRY(IVideoFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CAvFrame)

public://IAvFrame
    AvFrame::AvFrameType getFrameType();

public://IVideoFrame
    Tensor getFrameVideoImage(VideoFrame::AvFrameImageType eType);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CVideoFrame_h__