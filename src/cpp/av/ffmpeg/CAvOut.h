#ifndef __SimpleWork_av_ffmpeg_CAvOut_h__
#define __SimpleWork_av_ffmpeg_CAvOut_h__

#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvOut : public CObject, public IAvOut {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int pubFrame(SAvFrame& rFrame);

public:
    CAvOut();
    ~CAvOut();
    void release();
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvOut_h__