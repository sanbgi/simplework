#ifndef __SimpleWork_av_ffmpeg_CAvOut_h__
#define __SimpleWork_av_ffmpeg_CAvOut_h__

#include <vector>
#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvOutStreaming;
class CAvOut : public CObject, public IAvOut {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvOut
    int writeFrame(const SAvFrame& avFrame);

public:
    static int createAvFile(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings, SAvOut& spAvOut);
    int initAvFile(const char* szFileName, int nStreamings, const PAvStreaming* pStreamings);

public:
    CAvOut();
    ~CAvOut();
    void release();
    int close();

private:
    CTaker<AVFormatContext*> m_spFormatContext;
    CTaker<AVIOContext*> m_spIOContext;
    std::vector<CPointer<CAvOutStreaming>> m_arrStreamings;
    bool m_bOpened;
    std::string m_strFileName;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvOut_h__