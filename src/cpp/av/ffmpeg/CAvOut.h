#ifndef __SimpleWork_av_ffmpeg_CAvOut_h__
#define __SimpleWork_av_ffmpeg_CAvOut_h__

#include <vector>
#include "av_ffmpeg.h"
#include "CAvStreaming.h"

FFMPEG_NAMESPACE_ENTER

class CAvOutStreaming;
class CAvOut : public CObject, public IAvOut, public IPipe {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvOut)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IPipe
    int pushData(const PData& rData, IVisitor<const PData&>* pReceiver);

public://IAvOut
    int pushFrame(const PAvFrame* pFrame);

public:
    int initAvFile(const char* szFileName, int nStreamings, PAvStreaming* pStreamings);

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