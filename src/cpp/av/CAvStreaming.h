#ifndef __SimpleWork_Av_CAvStream_h__
#define __SimpleWork_Av_CAvStream_h__

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
}

using namespace sw::core;
using namespace sw::av;

class CAvStreaming : public CObject, IAvStreaming {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvStreaming)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    AvStreaming::AvStreamingType getStreamingType();
    int getStreamingIndex();

public:
    int init(AVStream* pAvStream, int iStreamingIndex);

public:
    CAvStreaming();
    ~CAvStreaming();

public:
    void release();

public:
    AVStream* m_pAvStream;  //CAvIn_ffmpeg::m_pFormatCtx持有，无需释放
    AVCodecContext* m_pCodecCtx;
    AvStreaming::AvStreamingType m_eAvStreamingType;
    int m_iStreamingIndex;
};

#endif//__SimpleWork_Av_CAvStream_h__