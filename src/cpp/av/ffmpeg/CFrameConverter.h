#ifndef __SimpleWork_Av_CImageConverter_h__
#define __SimpleWork_Av_CImageConverter_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CFrameConverter : public CObject {

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

public:
    int init(AVPixelFormat eTargetPixFmt);

public://Image
    CTaker<SwsContext*> m_spSwsContext;
    uint8_t *m_pVideoData[AV_NUM_DATA_POINTERS];
    int m_pVideoLinesizes[AV_NUM_DATA_POINTERS];

public://Audio
    CTaker<SwrContext*> m_spSwrCtx;
    uint8_t **m_ppAudioData;
    int m_nAudioSamples;

public:

    CFormat m_lastTargetFormat;
    CFormat m_lastSourceFormat;

public:
    CFrameConverter();
    ~CFrameConverter();
    void releaseVideoCtx();
    void releaseAudioCtx();
    void releaseVideoData();
    void releaseAudioData();

public:
    int convert(int targetWidth, int targetHeight, AVPixelFormat targetFormat, AVFrame& src);
    int convert(int nTargetRate, int nTargetChannels, AVSampleFormat targetFormat, AVFrame& src);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_Av_CImageConverter_h__