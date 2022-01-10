
#include "av_ffmpeg.h"
#include "CAvFrame.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

static SCtx sCtx("CAvFrame");

AVFrame* CAvFrame::allocAvFramePtr(AVStream* pStreaming, int iStringingId) {
    m_spAvFrame.take(av_frame_alloc(), [](AVFrame* pFrame){
        av_frame_free(&pFrame);
    });
    AVFrame* pAvFrame = m_spAvFrame;
    AVCodecParameters * pCodecCtx = pStreaming->codecpar;
    switch (pCodecCtx->codec_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
            m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVPixelFormat)pCodecCtx->format);
            m_avFrame.sampleMeta.videoWidth = pCodecCtx->width;
            m_avFrame.sampleMeta.videoHeight = pCodecCtx->height;
        }
        break;
    
    case AVMEDIA_TYPE_AUDIO:
        {
            m_avFrame.sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
            m_avFrame.sampleMeta.sampleFormat = CAvSampleType::convert((AVSampleFormat)pCodecCtx->format);
            m_avFrame.sampleMeta.audioRate = pCodecCtx->sample_rate;
            m_avFrame.sampleMeta.audioChannels = pCodecCtx->channels;
        }
        break;

    default:
        sCtx.error("未知的帧类型");
        return nullptr;
    }
    AVRational r = pStreaming->time_base;
    m_avFrame.timeRate = r.den/r.num;
    m_avFrame.streamingId = iStringingId;
    return pAvFrame;
}

int CAvFrame::setAVFrameToPAvFrame() {
    if(!m_spAvFrame) {
        return sCtx.error("没有帧信息，无法更新");
    }

    AVFrame* pAvFrame = m_spAvFrame;
    PAvFrame& avFrame = m_avFrame;
    
    avFrame.ppPlanes = pAvFrame->data;
    avFrame.pPlaneLineSizes = pAvFrame->linesize;

    //
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据，这里面要注意，是否存在
    //  planar audio，并且通道数超过8？如果存在这种情况，则这里的数据是存在丢失的
    //
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pAvFrame->data[i]; i++ ) {
        nPlanes = i+1;
    }

    avFrame.nPlanes = nPlanes;
    avFrame.timeStamp = pAvFrame->pts;
    switch(m_avFrame.sampleMeta.sampleType) {
    case EAvSampleType::AvSampleType_Audio:
        {
            avFrame.nWidth = pAvFrame->nb_samples;
            avFrame.nHeight = 1;
        }
        break;

    case EAvSampleType::AvSampleType_Video:
        {
            avFrame.nWidth = pAvFrame->width;
            avFrame.nHeight = pAvFrame->height;
        }
        break;

    default:
        {
            return sCtx.error("暂时没有支持的帧类型");
        }
    }
    return sCtx.success();
}

const PAvFrame* CAvFrame::getFramePtr() {

    return &m_avFrame;
}

FFMPEG_NAMESPACE_LEAVE