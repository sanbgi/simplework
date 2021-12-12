#ifndef __SimpleWork_Av_CAvFrame_h__
#define __SimpleWork_Av_CAvFrame_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvStreaming;
class CAvFrame : public CObject, public IAvFrame {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    AvStreaming& getStreaming();

public:
    static int createAvFrame(CFFMpegPointer<AVFrame>& rAvFrame, CAvStreaming* pStreaming, AvFrame& rFrame); 

public:
    CAvFrame();
    ~CAvFrame();

public:
    AVFrame* m_pAvFrame;
    AvStreaming m_spAvStream;
    CAvStreaming* m_pStreaming;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvFrame_h__