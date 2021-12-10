#ifndef __SimpleWork_Av_CAvFrame_h__
#define __SimpleWork_Av_CAvFrame_h__

extern "C" {
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #include <libavutil/imgutils.h>
    #include <SDL2/SDL.h>
}

using namespace sw::core;
using namespace sw::math;
using namespace sw::av;

class CAvFrame : public CObject, IAvFrame {
    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    AvStreaming::AvStreamingType getStreamingType();
    AvStreaming& getStreaming();
    Tensor getVideoImage(AvFrame::AvFrameImageType eType);

public:
    void attachAvFrame(AVFrame* pAvFrame);

public:
    CAvFrame();
    ~CAvFrame();

public:
    AVFrame* m_pAvFrame;
    AvFrame::AvFrameType m_eAvFrameType;
    AvStreaming m_spAvStream;
    AVCodecContext* m_pCodecCtx;
    NamedMap m_mapCtx;
};

#endif//__SimpleWork_Av_CAvFrame_h__