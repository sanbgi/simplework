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

EAvStreamingType CAvStreaming::getStreamingType() {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return EAvStreamingType::AvStreamingType_Video;
        
        case AVMEDIA_TYPE_AUDIO:
            return EAvStreamingType::AvStreamingType_Audio;
        }
    }
    return EAvStreamingType::AvStreamingType_None;
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

EAvSampleType CAvStreaming::getSampleType() {
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            EAvSampleType eType = CAvSampleType::convert(m_spCodecCtx->pix_fmt);
            if(eType == EAvSampleType::AvSampleType_None) {
                eType = EAvSampleType::AvSampleType_Video_RGBA;
            }
            return eType;
        }
        break;

    case AVMEDIA_TYPE_AUDIO:
        {
            EAvSampleType eType = CAvSampleType::convert(m_spCodecCtx->sample_fmt);
            if(eType == EAvSampleType::AvSampleType_None) {
                eType = EAvSampleType::AvSampleType_Audio_S16;
            }
            return eType;
        }
        break;
    }
    return EAvSampleType::AvSampleType_None;
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
    m_sampleMeta.sampleType = getSampleType();
    switch(m_spCodecCtx->codec_type) {
    case AVMEDIA_TYPE_VIDEO:
        {
            m_sampleMeta.videoWidth = m_spCodecCtx->width;
            m_sampleMeta.videoHeight = m_spCodecCtx->height;
        }
        break;
    case AVMEDIA_TYPE_AUDIO:
        {
            m_sampleMeta.audioRate = m_spCodecCtx->sample_rate;
            m_sampleMeta.audioChannels = m_spCodecCtx->channels;
        }
        break;
    }

    SObject spFilter;
    CAvFilter* pAvFilter = CObject::createObject<CAvFilter>(spFilter);
    if( pAvFilter->initFilter(m_sampleMeta) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }
    m_spFilter.take(pAvFilter, spFilter);
    return SError::ERRORTYPE_SUCCESS;
}


int CAvStreaming::receiveFrame(PAvFrame::FVisitor receiver, AVFrame* pAvFrame) {
    PAvFrame avFrame;
    avFrame.sampleMeta = m_sampleMeta;
    avFrame.planeDatas = pAvFrame->data;
    avFrame.planeLineSizes = pAvFrame->linesize;
    avFrame.streamingId = getStreamingId();
    avFrame.streamingType = getStreamingType();
    avFrame.samples = pAvFrame->nb_samples;
    avFrame.timeRate = getTimeRate();
    avFrame.timeStamp = pAvFrame->pts;
    switch(avFrame.streamingType) {
        case EAvStreamingType::AvStreamingType_Video:
            return m_spFilter->convertVideo((AVPixelFormat)pAvFrame->format, &avFrame, receiver);

        case EAvStreamingType::AvStreamingType_Audio:
            avFrame.planeDatas = pAvFrame->extended_data;
            return m_spFilter->convertAudio((AVSampleFormat)pAvFrame->format, &avFrame, receiver);
    }
    return SError::ERRORTYPE_FAILURE;
}

FFMPEG_NAMESPACE_LEAVE