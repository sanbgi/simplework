#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

#include "av_ffmpeg.h"
#include "CAvFrameConverter.h"

FFMPEG_NAMESPACE_ENTER

class CAvStreaming : public CObject {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        //SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    EAvSampleType getSampleType();
    int getStreamingId();
    int getTimeRate();
    long getDuration();
    int getSampleRate();
    EAvSampleFormat getSampleFormat();
    const PAvSample& getSampleMeta();

public:
    int init(AVStream* pAvStream, int iStreamingIndex);
    int setSampleMeta(const PAvSample& sampleMeta);
    int receiveFrame(PAvFrame::FVisitor receiver, AVFrame* pAvFrame);

public:
    CAvStreaming();
    ~CAvStreaming();

public:
    void release();
    
public:
    AVStream* m_pAvStream;  //CAvIn_ffmpeg::m_pFormatCtx持有，无需释放
    CTaker<AVCodecContext*> m_spCodecCtx;
    int m_iStreamingIndex;
    PAvSample m_sampleMeta;

public:
    //SAvFrameConverter m_spFilter;
    SPipe m_spConverter;

public://Video
    int m_lastDimsize[3];
    STensor m_spLastDimTensor;
    
public:

};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CAvStream_h__