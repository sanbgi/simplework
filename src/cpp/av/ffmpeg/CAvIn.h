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
    int getWidth();
    int getHeight();
    int getStreaming(SAvStreaming& rStreaming);
    int getFrame(SAvFrame& rFrame);

public:
    int initVideoFile(const char* szFileName);
    int initVideoCapture(const char* szName);
    int initAudioCapture(const char* szName);
    int sendPackageAndReceiveFrame(SAvFrame& frame, AVPacket* pPackage);
    int receiveFrame(SAvFrame& frame, CAvStreaming* pStreaming);
    int initCapture(AVInputFormat* pInputForamt, const char* szName);
    static void initDeviceRegistry();

public:
    CAvIn();
    ~CAvIn();
    void release();

private:
    CTaker<AVFormatContext*> m_spFormatCtx;
    CTaker<AVFormatContext*> m_spOpenedCtx;
    CAvStreaming* m_pContinueReadingStreaming;
    std::vector<CAvStreaming*> m_vecCAvStreamings;
    std::vector<SAvStreaming> m_vecAvStreamings;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvIn_h__