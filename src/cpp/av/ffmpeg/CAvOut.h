#ifndef __SimpleWork_av_ffmpeg_CAvOut_h__
#define __SimpleWork_av_ffmpeg_CAvOut_h__

#include <vector>
#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvOutStreaming;
class CAvOut : public CObject, public IAvOut {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int initAvFile(const char* szFileName, int nStreamings, SAvStreaming* pStreamings);

public:
    int writeFrame(const SAvFrame& rFrame);
    int writeFrame(const PAvFrame* pFrame);
    int close();

public:
    CAvOut();
    ~CAvOut();
    void release();

private:
    CTaker<AVFormatContext*> m_spFormatContext;
    CTaker<AVIOContext*> m_spIOContext;
    std::vector<CPointer<CAvOutStreaming>> m_arrStreamings;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvOut_h__