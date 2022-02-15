#ifndef __SimpleWork_av_ffmpeg_CAvFrame_h__
#define __SimpleWork_av_ffmpeg_CAvFrame_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvFrame : public CObject, public IAvFrame {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    const PAvFrame* getFramePtr();

public:
    AVFrame* allocAvFramePtr(AVStream* pStreaming, int iStringingId);
    int setAVFrameToPAvFrame();

public:
    static int loadImage(const char* szFileName, SAvFrame& spFrame);
    static int saveImage(const char* szFileName, const SAvFrame& spFrame);

public:
    CTaker<AVFrame*> m_spAvFrame;
    CTaker<uint8_t**> m_spPlanes;
    uint8_t *m_ppPlanes[AV_NUM_DATA_POINTERS];
    int m_pLinesizes[AV_NUM_DATA_POINTERS];
    PAvFrame m_avFrame;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvFrame_h__