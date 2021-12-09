#include "Av.h"
#include "CAvStreaming.h"

CAvStreaming::CAvStreaming() {
    m_eAvStreamingType = AvStreaming::AVSTREAMTYPE_UNKNOWN;
    m_pCodecCtx = nullptr;
    m_iStreamingIndex = -1;
}

CAvStreaming::~CAvStreaming() {
    release();
}

void CAvStreaming::release() {
    if(m_pCodecCtx) {
        avcodec_free_context(&m_pCodecCtx);
        m_pCodecCtx = nullptr;
    }
}

AvStreaming::AvStreamingType CAvStreaming::getStreamingType() {
    if(m_pCodecCtx) {
        switch (m_pCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return AvStreaming::AVSTREAMTYPE_VIDEO;
        
        case AVMEDIA_TYPE_AUDIO:
            return AvStreaming::AVSTREAMTYPE_AUDIO;
        }
    }
    return AvStreaming::AVSTREAMTYPE_UNKNOWN;
}

int CAvStreaming::getStreamingIndex() {
    return m_iStreamingIndex;
}

int CAvStreaming::init(AVStream* pAvStream, int iStreamingIndex) {
    AVCodecParameters* pCodecParameter = pAvStream->codecpar;
    if(pCodecParameter == nullptr) {
        return Error::ERRORTYPE_FAILURE;
    }

    CAutoFree<AVCodecContext> pCodecCtx(avcodec_alloc_context3(nullptr), avcodec_free_context);
    if( avcodec_parameters_to_context(pCodecCtx, pCodecParameter) < 0 ) {
        return Error::ERRORTYPE_FAILURE;
    }

    AVCodec* pCodec=avcodec_find_decoder(pCodecParameter->codec_id);
    if(pCodec==NULL){
        printf("Codec not found.\n");
        return Error::ERRORTYPE_FAILURE;
    }

    if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
        printf("Could not open codec.\n");
        return Error::ERRORTYPE_FAILURE;
    }

    m_pCodecCtx = pCodecCtx.detach();
    m_iStreamingIndex = iStreamingIndex;
    return Error::ERRORTYPE_SUCCESS;
}