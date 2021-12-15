#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvStreaming : public CObject, IAvStreaming {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    EAvStreamingType getStreamingType();
    int getStreamingId();
    int getSampleRate();
    EAvSampleType getSampleType();
    const CAvSampleMeta& getSampleMeta();

public:
    int init(AVStream* pAvStream, int iStreamingIndex);
    int setSampleMeta(const CAvSampleMeta& sampleMeta);
    int convertToTensor(STensor& spData, AVFrame* pAvFrame);
    int convertAudio(STensor& spData, AVFrame* pAvFrame);
    int convertImage(STensor& spData, AVFrame* pAvFrame);

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
    EAvStreamingType m_eAvStreamingType;
    int m_iStreamingIndex;

public://Audio
    CTaker<SwrContext*> m_spSwrCtx;

public://Video
    CTaker<SwsContext*> m_spSwsContext;
    uint8_t *m_pData[4];
    int m_pLinesizes[4];
    int m_nPixelBytes;

public:
    CAvSampleMeta m_sampleMeta;
    CAvSampleMeta m_lastMeta;
    STensor m_spLastDimTensor;
    int m_nLastSamples;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvStream_h__