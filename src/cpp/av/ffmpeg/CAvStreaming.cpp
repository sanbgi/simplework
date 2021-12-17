#include "av_ffmpeg.h"
#include "CAvStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

CAvStreaming::CAvStreaming() {
    m_pAvStream = nullptr;
    m_iStreamingIndex = -1;
}

CAvStreaming::~CAvStreaming() {
    release();
}

void CAvStreaming::release() {
}

EAvSampleType CAvStreaming::getSampleType() {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return EAvSampleType::AvSampleType_Video;
        
        case AVMEDIA_TYPE_AUDIO:
            return EAvSampleType::AvSampleType_Audio;
        }
    }
    return EAvSampleType::AvSampleType_None;
}

int CAvStreaming::getStreamingId() {
    return m_iStreamingIndex;
}

int CAvStreaming::getTimeRate() {
    AVRational r = m_pAvStream->time_base;
    return r.den/r.num;
}

int CAvStreaming::getSampleRate() {
    return m_spCodecCtx->sample_rate;
}

EAvSampleFormat CAvStreaming::getSampleFormat() {
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            EAvSampleFormat eType = CAvSampleType::convert(m_spCodecCtx->pix_fmt);
            if(eType == EAvSampleFormat::AvSampleFormat_None) {
                eType = EAvSampleFormat::AvSampleFormat_Video_RGBA;
            }
            return eType;
        }
        break;

    case AVMEDIA_TYPE_AUDIO:
        {
            EAvSampleFormat eType = CAvSampleType::convert(m_spCodecCtx->sample_fmt);
            if(eType == EAvSampleFormat::AvSampleFormat_None) {
                eType = EAvSampleFormat::AvSampleFormat_Audio_S16;
            }
            return eType;
        }
        break;
    }
    return EAvSampleFormat::AvSampleFormat_None;
}

const PAvSample& CAvStreaming::getSampleMeta() {
    return m_sampleMeta;
}

int CAvStreaming::setSampleMeta(const PAvSample& sampleMeta) {
    m_sampleMeta = sampleMeta;
    return SError::ERRORTYPE_SUCCESS;
}

int CAvStreaming::init(AVStream* pAvStream, int iStreamingIndex) {
    AVCodecParameters* pCodecParameter = pAvStream->codecpar;
    if(pCodecParameter == nullptr) {
        return SError::ERRORTYPE_FAILURE;
    }

    CTaker<AVCodecContext*> pCodecCtx(  avcodec_alloc_context3(nullptr), 
                                        [](AVCodecContext* pCtx){
                                            avcodec_free_context(&pCtx);
                                        });
    if( avcodec_parameters_to_context(pCodecCtx, pCodecParameter) < 0 ) {
        return SError::ERRORTYPE_FAILURE;
    }

    AVCodec* pCodec=avcodec_find_decoder(pCodecParameter->codec_id);
    if(pCodec==NULL){
        printf("Codec not found.\n");
        return SError::ERRORTYPE_FAILURE;
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
        printf("Could not open codec.\n");
        return SError::ERRORTYPE_FAILURE;
    }

    m_spCodecCtx.take(pCodecCtx);
    m_iStreamingIndex = iStreamingIndex;
    m_pAvStream = pAvStream;
    m_sampleMeta.sampleFormat = getSampleFormat();
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_sampleMeta.videoWidth = m_spCodecCtx->width;
            m_sampleMeta.videoHeight = m_spCodecCtx->height;
            m_sampleMeta.sampleType = EAvSampleType::AvSampleType_Video;
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        {
            m_sampleMeta.audioRate = m_spCodecCtx->sample_rate;
            m_sampleMeta.audioChannels = m_spCodecCtx->channels;
            m_sampleMeta.sampleType = EAvSampleType::AvSampleType_Audio;
        }
        break;
    }

    if( SAvFilter::createFilter(m_sampleMeta, m_spFilter) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }

    return SError::ERRORTYPE_SUCCESS;
}


int CAvStreaming::receiveFrame(PAvFrame::FVisitor receiver, AVFrame* pAvFrame) {
    PAvFrame avFrame;
    avFrame.sampleMeta = m_sampleMeta;
    // 通过搜索linesize里面的值，来判断究竟有多少plane, 便于处理数据
    int nPlanes = 0;
    for( int i=0; i<AV_NUM_DATA_POINTERS && pAvFrame->data[i]; i++ ) {
        nPlanes = i+1;
    }
    avFrame.samplePlanes = nPlanes;
    avFrame.planeDatas = pAvFrame->data;
    avFrame.planeLineSizes = pAvFrame->linesize;
    avFrame.streamingId = getStreamingId();
    avFrame.samples = pAvFrame->nb_samples;
    avFrame.timeRate = getTimeRate();
    avFrame.timeStamp = pAvFrame->pts;
    return m_spFilter->putFrame(&avFrame, receiver);
}

FFMPEG_NAMESPACE_LEAVE