#ifndef __SimpleWork_Av_CAvOutStreaming_h__
#define __SimpleWork_Av_CAvOutStreaming_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvOutStreaming : public CObject, IVisitor<const PAvFrame*> {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    EAvSampleType getSampleType();
    int getStreamingId();
    int getTimeRate();
    long getDuration();
    const PAvSample& getSampleMeta();

public:
    int init(AVFormatContext* pFormatContext, const PAvStreaming* pSrc);
    int initVideo(AVFormatContext* pFormatContext, const PAvStreaming* pSrc);
    int initAudio(AVFormatContext* pFormatContext, const PAvStreaming* pSrc);
    int open(AVFormatContext* pFormatContext);
    int close(AVFormatContext* pFormatContext);
    int pushFrame(AVFormatContext* pFormatContext, const PAvFrame* pFrame);
    int writeVideoFrame(AVFormatContext* pFormatContext, const PAvFrame* pFrame);
    int writeAudioFrame(AVFormatContext* pFormatContext, const PAvFrame* pFrame);
    int writeFrame(AVFormatContext* pFormatContext, AVFrame* pFrame);
    int visit(const PAvFrame* pFrame);

public:
    CAvOutStreaming();
    ~CAvOutStreaming();

public:
    void release();

public:
    EAvSampleType m_eStreamingType;
    int m_iStreamingId;
    int m_iStreamingIndex;
    int m_nTimeRate;
    long m_nDuration;
    int m_nWriteNumber;
    PAvSample m_sampleMeta;
    
    AVStream* m_pAvStream;
    CTaker<AVFrame*> m_pAVFrame;
    CTaker<AVCodecContext*> m_spCodecCtx;
    AVCodec* m_pCodec;
    SAvFrameConverter m_spFilter;
    AVFormatContext* m_pFormatContext;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvOutStreaming_h__