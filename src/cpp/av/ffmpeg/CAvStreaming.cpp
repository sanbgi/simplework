#include "av_ffmpeg.h"
#include "CAvStreaming.h"
#include "CAvSampleType.h"

FFMPEG_NAMESPACE_ENTER

CAvStreaming::CAvStreaming() {
    m_pAvStream = nullptr;
    m_eAvStreamingType = EAvStreamingType::AvStreamingType_None;
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
    return SError::ERRORTYPE_SUCCESS;
}

int CAvStreaming::convertToTensor(STensor& spData, AVFrame* pAvFrame) {
    if(m_spCodecCtx) {
        switch (m_spCodecCtx->codec_type)
        {
        case AVMEDIA_TYPE_VIDEO:
            return convertImage(spData, pAvFrame);
        
        case AVMEDIA_TYPE_AUDIO:
            return convertAudio(spData, pAvFrame);
        }
    }
    return SError::ERRORTYPE_FAILURE;
}

int CAvStreaming::convertAudio(STensor& spTensor, AVFrame* pAvFrame) {
    PAvSample& sampleMeta = m_sampleMeta;
    int audioRate = m_sampleMeta.audioRate > 0 ? m_sampleMeta.audioRate : pAvFrame->sample_rate;
    int nChannels = m_sampleMeta.audioChannels > 0 ? m_sampleMeta.audioChannels : pAvFrame->channels;
    AVSampleFormat eSampleFormat = CAvSampleType::toSampleFormat(sampleMeta.sampleType);
    if( m_converter.convert(audioRate,nChannels,eSampleFormat, *pAvFrame) != SError::ERRORTYPE_SUCCESS) {
        return SError::ERRORTYPE_FAILURE;
    }

    // 重采样返回的一帧音频数据大小(以字节为单位)
    int nBytesPerSample = av_get_bytes_per_sample(eSampleFormat);
    int dimsize[3] = { nChannels, m_converter.m_nAudioSamples, nBytesPerSample };
    if( !m_spLastDimTensor ) {
        m_spLastDimTensor = STensor::createVector(3, dimsize);
    }
    if( m_spLastDimTensor ) {
        if( dimsize[0] != m_lastDimsize[0] ||
            dimsize[1] != m_lastDimsize[1] ||
            dimsize[2] != m_lastDimsize[2]) {
            m_spLastDimTensor.release();
            m_spLastDimTensor = STensor::createVector(3, dimsize);
            m_lastDimsize[0] = dimsize[0];
            m_lastDimsize[1] = dimsize[1];
            m_lastDimsize[2] = dimsize[2];
        }
    }else{
        m_spLastDimTensor = STensor::createVector(3, dimsize);
        m_lastDimsize[0] = dimsize[0];
        m_lastDimsize[1] = dimsize[1];
        m_lastDimsize[2] = dimsize[2];
    }

    int nData = nChannels * m_converter.m_nAudioSamples * nBytesPerSample;
    spTensor = STensor::createTensor(m_spLastDimTensor, nData, m_converter.m_ppAudioData[0]);
    return SError::ERRORTYPE_FAILURE;
}

int CAvStreaming::convertImage(STensor& spTensor, AVFrame* pAvFrame) {
    AVPixelFormat eTargetPixelFormat = CAvSampleType::toPixFormat(m_sampleMeta.sampleType);
    int nTargetWidth = m_sampleMeta.videoWidth > 0 ? m_sampleMeta.videoWidth : pAvFrame->width;
    int nTargetHeight = m_sampleMeta.videoHeight > 0 ? m_sampleMeta.videoHeight : pAvFrame->height;
    if( m_converter.convert(nTargetWidth, nTargetHeight, eTargetPixelFormat, *pAvFrame) != SError::ERRORTYPE_SUCCESS ) {
        return SError::ERRORTYPE_FAILURE;
    }
    
    int nPixBytes = 0;
    switch(m_sampleMeta.sampleType) {
    case EAvSampleType::AvSampleType_Video_RGBA:
        nPixBytes = 4;
        break;

    case EAvSampleType::AvSampleType_Video_RGB:
        nPixBytes = 3;
        break;

    default:
        return SError::ERRORTYPE_FAILURE;
    }

    int dimsize[3] = { nTargetHeight, nTargetWidth, nPixBytes };
    if( m_spLastDimTensor ) {
        if( dimsize[0] != m_lastDimsize[0] ||
            dimsize[1] != m_lastDimsize[1] ||
            dimsize[2] != m_lastDimsize[2]) {
            m_spLastDimTensor.release();
            m_spLastDimTensor = STensor::createVector(3, dimsize);
            m_lastDimsize[0] = dimsize[0];
            m_lastDimsize[1] = dimsize[1];
            m_lastDimsize[2] = dimsize[2];
        }
    }else{
        m_spLastDimTensor = STensor::createVector(3, dimsize);
        m_lastDimsize[0] = dimsize[0];
        m_lastDimsize[1] = dimsize[1];
        m_lastDimsize[2] = dimsize[2];
    }
    
    //
    //  修正视频宽度。及nTargetWidth != m_pVideoLinesizes[0]/4时，以后者为准。如果返
    //  回值中的视频，一行的字节数不等于视频宽度*4(RGBA)，则这个时候视频数据有点难处
    //  理，只能暂时将返回图像的宽度扩大(有垃圾数据)或缩小(数据丢失）到返回的实际宽度。
    //  如果想调整为实际视频大小，则需要对每一行的数据做处理，性能太低。
    //
    spTensor = STensor::createTensor(m_spLastDimTensor, nTargetWidth*nTargetHeight*nPixBytes, m_converter.m_pVideoData[0]);
    return SError::ERRORTYPE_SUCCESS;
}

FFMPEG_NAMESPACE_LEAVE