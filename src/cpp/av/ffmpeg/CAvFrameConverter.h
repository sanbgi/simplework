#ifndef __SimpleWork_av_ffmpeg_CAvFrameConverter_h__
#define __SimpleWork_av_ffmpeg_CAvFrameConverter_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvFrameConverter : public CObject, public IAvNetwork {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvNetwork)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public:
    int pipeIn(const SAvFrame& spIn, SAvFrame& spOut);

public://For factory
    static int createFilter(const PAvSample& targetSample, SAvNetwork& spFilter);

private:
    class CFormat {
    public:
        CFormat(){}
        CFormat(int nWidth, int nHeight, AVPixelFormat eFormat) {
            m_nWidth = nWidth;
            m_nHeight = nHeight;
            m_nFormat = eFormat;
        }

        CFormat(int nRate, int nChannels, AVSampleFormat eFormat) {
            m_nRate = nRate;
            m_nChannels = nChannels;
            m_nFormat = eFormat;
        }

        bool operator != (const CFormat& src) {
            return  m_nFormat != src.m_nFormat ||
                    m_nWidth != src.m_nWidth ||
                    m_nHeight != src.m_nHeight;
        }

        union {
            int m_nWidth;
            int m_nRate;
        };

        union {
            int m_nHeight;
            int m_nChannels;
        };

        int m_nFormat;
    };

private://Image
    CTaker<SwsContext*> m_spSwsContext;

private://Audio
    CTaker<SwrContext*> m_spSwrCtx;

private:
    PAvSample m_targetSample;
    CFormat m_targetFormat;
    CFormat m_lastSourceFormat;

public:
    CAvFrameConverter();
    ~CAvFrameConverter();
    
    void release();
    void releaseVideoCtx();
    void releaseAudioCtx();

private:
    int initFilter(const PAvSample& targetSample);

private:
    int convertAudio(const SAvFrame& spIn, SAvFrame& spOut);
    int convertVideo(const SAvFrame& spIn, SAvFrame& spOut);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvFrameConverter_h__