#ifndef __SimpleWork_av_ffmpeg_CAvIn_h__
#define __SimpleWork_av_ffmpeg_CAvIn_h__

#include "av_ffmpeg.h"
#include "CAvInStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvIn : public CObject, public IAvIn {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvIn)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvIn
    int getStreamingSize();
    const PAvStreaming* getStreamingAt(int iPos);
    int readFrame(SAvFrame& spAvFrame);
    bool isCompleted();

public:
    static int createAvFileReader(const char* szFileName, SAvIn& spIn);
    int initVideoFile(const char* szFileName);
    int initVideoCapture(const char* szName);
    int initAudioCapture(const char* szName);
    int sendPackageAndReceiveFrame(SAvFrame& spAvFrame, AVPacket* pPackage);
    int receiveFrame(SAvFrame& spAvFrame, CAvInStreaming* pStreaming);
    int initCapture(AVInputFormat* pInputForamt, const char* szName);
    static void initDeviceRegistry();

public:
    CAvIn();
    ~CAvIn();
    void release();

private:
    CTaker<AVFormatContext*> m_spFormatCtx;
    CTaker<AVFormatContext*> m_spOpenedCtx;
    std::vector<CAvInStreaming*> m_arrToReadingStreamings;
    std::vector<CPointer<CAvInStreaming>> m_arrAvStreamings;
    std::vector<PAvStreaming> m_arrStreamings;
    int m_isCompoeted;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvIn_h__