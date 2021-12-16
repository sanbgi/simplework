#ifndef __SimpleWork_Av_CAvOutStreaming_h__
#define __SimpleWork_Av_CAvOutStreaming_h__

#include "av_ffmpeg.h"
#include "CFrameConverter.h"

FFMPEG_NAMESPACE_ENTER

class CAvOutStreaming : public CObject, IAvStreaming {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    EAvStreamingType getStreamingType();
    int getStreamingId();
    int getTimeRate();
    const CAvSampleMeta& getSampleMeta();

public:
    int init(AVFormatContext* pFormatContext, SAvStreaming& src);
    int initVideo(AVFormatContext* pFormatContext, SAvStreaming& src);
    int initAudio(AVFormatContext* pFormatContext, SAvStreaming& src);
    int open(AVFormatContext* pFormatContext);
    int close(AVFormatContext* pFormatContext);
    int writeFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame);
    int writeVideoFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame);
    int writeAudioFrame(AVFormatContext* pFormatContext, const SAvFrame& rFrame);
    int writeFrame(AVFormatContext* pFormatContext, AVFrame* pFrame);

public:
    CAvOutStreaming();
    ~CAvOutStreaming();

public:
    void release();

public:
    EAvStreamingType m_eStreamingType;
    int m_iStreamingId;
    int m_iStreamingIndex;
    int m_nTimeRate;
    int m_nWriteNumber;
    CAvSampleMeta m_sampleMeta;
    
    AVStream* m_pAvStream;
    CTaker<AVFrame*> m_pAVFrame;
    CTaker<AVCodecContext*> m_spCodecCtx;
    AVCodec* m_pCodec;
    CFrameConverter m_converter;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvOutStreaming_h__