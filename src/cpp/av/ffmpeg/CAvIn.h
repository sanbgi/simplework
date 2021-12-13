#ifndef __SimpleWork_av_ffmpeg_CAvIn_h__
#define __SimpleWork_av_ffmpeg_CAvIn_h__

#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvIn : public CObject, public IAvIn {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvIn)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int getStreamingCount();
    AvStreaming getStreaming(int iStreamingIndex);
    int getWidth();
    int getHeight();
    int getFrame(AvFrame& frame);

public:
    int initVideoFile(const char* szFileName);
    int initVideoCapture(const char* szName);
    int sendPackageAndReceiveFrame(AvFrame& frame, AVPacket* pPackage);
    int receiveFrame(AvFrame& frame, CAvStreaming* pStreaming);

public:
    CAvIn();
    ~CAvIn();
    void release();

private:
    bool m_bOpenedFormatCtx;
    CTaker<AVFormatContext*> m_spFormatCtx;
    CAvStreaming* m_pContinueReadingStreaming;
    std::vector<CAvStreaming*> m_vecCAvStreamings;
    std::vector<AvStreaming> m_vecAvStreamings;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvIn_h__