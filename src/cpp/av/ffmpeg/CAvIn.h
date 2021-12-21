#ifndef __SimpleWork_av_ffmpeg_CAvIn_h__
#define __SimpleWork_av_ffmpeg_CAvIn_h__

#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvIn : public CObject, public IPipe {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IPipe
    int pushData(const PData& rData, IVisitor<const PData&>* pReceiver);

public://IAvIn
    int changeStreamingSampleMeta(int iStreamingId, const PAvSample& sampleMeta);
    int readFrame(PAvFrame::FVisitor visitor);
    int visitStreamings(PAvStreaming::FVisitor visitor);


public:
    int initVideoFile(const char* szFileName);
    int initVideoCapture(const char* szName);
    int initAudioCapture(const char* szName);
    int sendPackageAndReceiveFrame(PAvFrame::FVisitor receiver, AVPacket* pPackage);
    int receiveFrame(PAvFrame::FVisitor receiver, CAvStreaming* pStreaming);
    int initCapture(AVInputFormat* pInputForamt, const char* szName);
    static void initDeviceRegistry();

public:
    CAvIn();
    ~CAvIn();
    void release();

private:
    CTaker<AVFormatContext*> m_spFormatCtx;
    CTaker<AVFormatContext*> m_spOpenedCtx;
    std::vector<CAvStreaming*> m_arrNeedReadingStreamings;
    std::vector<CPointer<CAvStreaming>> m_arrAvStreamings;
    int m_bHeaderReaded;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvIn_h__