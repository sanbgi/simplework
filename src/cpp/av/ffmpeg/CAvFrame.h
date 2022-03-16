#ifndef __SimpleWork_av_ffmpeg_CAvFrame_h__
#define __SimpleWork_av_ffmpeg_CAvFrame_h__

#include "av_ffmpeg.h"
#include <vector>

FFMPEG_NAMESPACE_ENTER

#define BUFFER_ALIGN    1
class CAvFrame : public CObject, public IAvFrame {

    SIMPLEWORK_INTERFACE_ENTRY_ENTER(CObject)
        SIMPLEWORK_INTERFACE_ENTRY(IAvFrame)
    SIMPLEWORK_INTERFACE_ENTRY_LEAVE(CObject)

public://IAvFrame
    const PAvFrame* getFramePtr();
    int getDataBuffer(SDeviceMemory& spDataBuffer);

private://
    int writeVideoFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame);
    int writeAudioSampleFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame);

public:
    static int createFrame(AVStream* pStreaming, int iStreamingId, AVFrame* pFrame, SAvFrame& spFrame);
    static int loadImage(const char* szFileName, SAvFrame& spFrame);
    static int saveImage(const char* szFileName, const SAvFrame& spFrame);
    static int allocImageDataBuffer(
                    SDeviceMemory& spDataBuffer, 
                    AVPixelFormat pixFormat, int nWidth, int nHeight, 
                    int pLinesizes[AV_NUM_DATA_POINTERS], 
                    uint8_t *ppPlanes[AV_NUM_DATA_POINTERS] );
    static int allocAudioSampleDataBuffer(
                    SDeviceMemory& spDataBuffer, 
                    AVSampleFormat sampleFormat, int nb_channels, int nb_samples,
                    int pLinesizes[AV_NUM_DATA_POINTERS], 
                    uint8_t *ppPlanes[AV_NUM_DATA_POINTERS] );

public:
    uint8_t *m_ppPlanes[AV_NUM_DATA_POINTERS];
    int m_pLinesizes[AV_NUM_DATA_POINTERS];
    PAvFrame m_avFrame;
    SDeviceMemory m_spBuffer;
};

FFMPEG_NAMESPACE_LEAVE

#endif//__SimpleWork_av_ffmpeg_CAvFrame_h__