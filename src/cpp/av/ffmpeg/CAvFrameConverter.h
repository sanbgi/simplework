#ifndef __SimpleWork_av_ffmpeg_CAvFrameConverter_h__
#define __SimpleWork_av_ffmpeg_CAvFrameConverter_h__

#include "av_ffmpeg.h"

FFMPEG_NAMESPACE_ENTER

class CAvFrameConverter : public CObject, public IAvFrameConverter, public IPipe {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFrameConverter)
        SIMPLEWORK_INTERFACE_ENTRY(IPipe)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IPipe
    int pushData(const PData& rData, IVisitor<const PData&>* pReceiver);

public://IAvFrameConverter
    int pushFrame(const PAvFrame* pSrc, PAvFrame::FVisitor visitor);

public://For factory
    static int createFilter(const PAvSample& targetSample, SAvFrameConverter& spFilter);
    static int createFilter(const PAvSample& targetSample, SPipe& spFilter);

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
    uint8_t *m_pVideoData[AV_NUM_DATA_POINTERS];
    int m_pVideoLinesizes[AV_NUM_DATA_POINTERS];

private://Audio
    CTaker<SwrContext*> m_spSwrCtx;
    uint8_t **m_ppAudioData;
    int m_pAudioLinesize[AV_NUM_DATA_POINTERS];
    int m_nAudioSamples;

private:
    int m_nPlanes;
    PAvSample m_targetSample;
    CFormat m_targetFormat;
    CFormat m_lastSourceFormat;

public:
    CAvFrameConverter();
    ~CAvFrameConverter();
    
    void release();
    void releaseVideoCtx();
    void releaseAudioCtx();
    void releaseVideoData();
    void releaseAudioData();

private:
    int initFilter(const PAvSample& targetSample);

private:
    int convertAudio(const PAvFrame* pSrc, PAvFrame::FVisitor visitor);
    int convertVideo(const PAvFrame* pSrc, PAvFrame::FVisitor visitor);
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvFrameConverter_h__