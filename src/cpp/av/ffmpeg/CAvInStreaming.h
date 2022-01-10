#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

#include "av_ffmpeg.h"
#include "CAvFrameConverter.h"

FFMPEG_NAMESPACE_ENTER

class CAvInStreaming : public CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        //SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int init(AVStream* pAvStream, int iStreamingIndex);
    PAvStreaming& getPAvStreaming();

public:
    CAvInStreaming();
    ~CAvInStreaming();

public:
    void release();
    
public:
    AVStream* m_pAvStream;  //CAvIn_ffmpeg::m_pFormatCtx持有，无需释放
    PAvStreaming m_avStreaming;
    CTaker<AVCodecContext*> m_spCodecCtx;
    bool m_isCompleted;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvStream_h__