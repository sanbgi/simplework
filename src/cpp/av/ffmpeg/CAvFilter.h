#ifndef __SimpleWork_av_ffmpeg_CAvFilter_h__
#define __SimpleWork_av_ffmpeg_CAvFilter_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvFilter : public CObject, public IAvFilter {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        //SIMPLEWORK_INTERFACE_ENTRY(IAvIn)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFilter
    int putFrame(const PAvFrame* pSrc, PAvFrame::FVisitor visitor);

public://For factory
    static int createFilter(const PAvSample& targetSample, SAvFilter& spFilter);

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

public://Image
    CTaker<SwsContext*> m_spSwsContext;
    uint8_t *m_pVideoData[AV_NUM_DATA_POINTERS];
    int m_pVideoLinesizes[AV_NUM_DATA_POINTERS];

public://Audio
    CTaker<SwrContext*> m_spSwrCtx;
    uint8_t **m_ppAudioData;
    int m_pAudioLinesize[AV_NUM_DATA_POINTERS];
    int m_nAudioSamples;

public:
    int m_nPlanes;
    PAvSample m_targetSample;
    CFormat m_targetFormat;
    CFormat m_lastSourceFormat;

public:
    CAvFilter();
    ~CAvFilter();
    
    void release();
    void releaseVideoCtx();
    void releaseAudioCtx();
    void releaseVideoData();
    void releaseAudioData();

public:
    int initFilter(const PAvSample& targetSample);
    int convertAudio(AVSampleFormat sourceFormat, const PAvFrame* pSrc, PAvFrame::FVisitor visitor);
    int convertVideo(AVPixelFormat sourceFormat, const PAvFrame* pSrc, PAvFrame::FVisitor visitor);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvFilter_h__