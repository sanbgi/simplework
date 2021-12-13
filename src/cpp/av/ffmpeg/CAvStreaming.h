#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvStreaming : public CObject, IAvStreaming {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    SAvFrame::AvFrameType getFrameType();
    int getStreamingId();

public:
    int init(AVStream* pAvStream, int iStreamingIndex);
    STensor convertAudio(AVFrame* pAvFrame, AVSampleFormat eSampleFormat, int nSampleRate, int nChannels);
    STensor convertImage(AVFrame* pAvFrame, AVPixelFormat ePixFormat);

public:
    CAvStreaming();
    ~CAvStreaming();

public:
    void release();
    void releaseAudioCtx();
    void releaseVideoCtx();

public:
    AVStream* m_pAvStream;  //CAvIn_ffmpeg::m_pFormatCtx持有，无需释放
    CTaker<AVCodecContext*> m_spCodecCtx;
    SAvFrame::AvFrameType m_eAvStreamingType;
    int m_iStreamingIndex;

public://Audio
    CTaker<SwrContext*> m_spSwrCtx;
    int m_nCtxSampleRate;
    int64_t m_nCtxChannels;
    AVSampleFormat m_eCtxSampleFormat;

public://Video
    CTaker<SwsContext*> m_spSwsContext;
    AVPixelFormat m_ePixFormat;
    uint8_t *m_pImagePointers[4];
    int m_pLinesizes[4];
    int m_nPixBytes;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvStream_h__