#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

#include "av_ffmpeg.h"
#include "CFrameConverter.h"

FFMPEG_NAMESPACE_ENTER

class CAvStreaming : public CObject, IAvStreaming {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    SAvStreaming::EAvStreamingType getStreamingType();
    int getStreamingId();
    int getTimeRate();
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
    
public:
    AVStream* m_pAvStream;  //CAvIn_ffmpeg::m_pFormatCtx持有，无需释放
    CTaker<AVCodecContext*> m_spCodecCtx;
    SAvStreaming::EAvStreamingType m_eAvStreamingType;
    int m_iStreamingIndex;
    CAvSampleMeta m_sampleMeta;

public:
    CFrameConverter m_converter;

public://Video
    int m_lastDimsize[3];
    STensor m_spLastDimTensor;
    
public:

};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvStream_h__